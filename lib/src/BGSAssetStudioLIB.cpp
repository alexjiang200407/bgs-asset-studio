#include "BGSAssetStudioLIB.h"
#include <nlohmann/json.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

using namespace std;
namespace fs = std::filesystem;

fs::path get_executable_directory()
{
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	return fs::path(buffer).parent_path();
}

LIBRARY_API const char* foo() { return "Hello World!"; }

LIBRARY_API void init_log()
{
	auto path = get_executable_directory() / "bgs-asset-studio.log";
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	auto logLevel = spdlog::level::trace;

	log->set_level(logLevel);
	log->flush_on(logLevel);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e] %v"s);
}

LIBRARY_API void stop_log()
{
	spdlog::info("Stopping log");

	spdlog::default_logger()->flush();
	spdlog::set_default_logger(nullptr);
	spdlog::drop_all();
	spdlog::shutdown();
}

#define ENUM_AND_STR(val) \
	{                     \
		#val, val         \
	}

static const std::unordered_map<std::string, DXGI_FORMAT> formatToStrMap = {
	ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_UNORM),    ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_UINT),
	ENUM_AND_STR(DXGI_FORMAT_B8G8R8A8_UNORM),    ENUM_AND_STR(DXGI_FORMAT_R32_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_D24_UNORM_S8_UINT),
};

#define ENUM_AND_STR(val) \
	{                     \
		val, #val         \
	}

static const std::unordered_map<DXGI_FORMAT, std::string> strToFormatMap = {
	ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_UNORM),    ENUM_AND_STR(DXGI_FORMAT_R8G8B8A8_UINT),
	ENUM_AND_STR(DXGI_FORMAT_B8G8R8A8_UNORM),    ENUM_AND_STR(DXGI_FORMAT_R32_FLOAT),
	ENUM_AND_STR(DXGI_FORMAT_D24_UNORM_S8_UINT),
};

DXGI_FORMAT dxgi_format_from_str(const std::string& str)
{
	auto it = formatToStrMap.find(str);
	if (it != formatToStrMap.end())
	{
		return it->second;
	}
	else
	{
		return DXGI_FORMAT_UNKNOWN;
	}
}

string dxgi_format_to_str(DXGI_FORMAT format)
{
	auto it = strToFormatMap.find(format);
	if (it != strToFormatMap.end())
	{
		return it->second;
	}
	else
	{
		return "DXGI_FORMAT_UNKNOWN";
	}
}

namespace ns
{
	struct tex_mapping_context
	{
		string      name;
		string      match_str;
		regex       match_regex;
		DXGI_FORMAT opaque_format;
		DXGI_FORMAT transparent_format;
	};

	using namespace nlohmann;
	void to_json(json& j, const tex_mapping_context& mapping)
	{
		j = json{ { "name", mapping.name },
			      { "regex", mapping.match_str },
			      { "dxgi-format-opaque", dxgi_format_to_str(mapping.opaque_format) },
			      { "dxgi-format-transparent", dxgi_format_to_str(mapping.transparent_format) } };
	}

	void from_json(const json& j, tex_mapping_context& tex_asset_type)
	{
		j.at("name").get_to(tex_asset_type.name);
		j.at("regex").get_to(tex_asset_type.match_str);

		tex_asset_type.match_regex = regex(tex_asset_type.match_str);

		string buf;
		j.at("dxgi-format-opaque").get_to(buf);

		tex_asset_type.opaque_format = dxgi_format_from_str(buf);

		j.at("dxgi-format-transparent").get_to(buf);
		tex_asset_type.transparent_format = dxgi_format_from_str(buf);
	}
}
using namespace nlohmann;
class asset_builder
{
public:
	void push(const fs::path& p)
	{
		ifstream ifs(p);
		json     js        = json::parse(ifs);
		json     tex_types = js.at("tex-types");

		vector<ns::tex_mapping_context> m(tex_types.size());

		if (tex_types.is_array())
		{
			size_t i = 0;
			for_e(field, tex_types)
			{
				ns::tex_mapping_context tex_asset;
				string                  name = field.at("name");

				m[i] = field.get_to(tex_asset);
				i++;
			}
		}

		path_stack.push(p);
		tex_mapping_context_stack.push(m);
	};
	void            pop() { path_stack.pop(); };
	const fs::path& top() { return path_stack.top(); };
	const string    asset_type(const fs::path& p)
	{
		const auto& current_level = tex_mapping_context_stack.top();
		for_e(entry, current_level)
		{
			if (regex_match(p.filename().string(), entry.match_regex))
			{
				return entry.name;
			}
		}
		return "unknown";
	};
	bool empty() { return path_stack.size() == 0; };

private:
	stack<fs::path>                        path_stack;
	stack<vector<ns::tex_mapping_context>> tex_mapping_context_stack;
};

void visit_directory(const fs::path& root)
{
	try
	{
		asset_builder builder;

		if (fs::exists(root / "bgs-asset-studio-meta.json"))
		{
			spdlog::info("Found new metadata file");
			builder.push(root / "bgs-asset-studio-meta.json");
		}

		int depth = 0;
		for (const auto& entry : fs::recursive_directory_iterator(root))
		{
			if (entry.is_directory())
			{
				spdlog::info(
					"Entered a new directory {} checking for {}",
					entry.path().string(),
					(entry.path() / "bgs-asset-studio-meta.json").string());
				if (fs::exists(entry.path() / "bgs-asset-studio-meta.json"))
				{
					spdlog::info("Found new metadata file");
					builder.push(entry.path() / "bgs-asset-studio-meta.json");
				}
			}

			if (!entry.is_directory())
			{
				spdlog::info(
					"using {} for {}",
					builder.empty() ? "default" : builder.top().string(),
					entry.path().string());
				spdlog::info("Is of type {}", builder.asset_type(entry.path()));
			}
		}
	}
	catch (const fs::filesystem_error& err)
	{
		std::cerr << "Error: " << err.what() << '\n';
	}
	catch (const exception& err)
	{
		spdlog::error("error: {}", err.what());
	}
}

LIBRARY_API void register_assets(filesystem::path dir) { visit_directory(dir); }