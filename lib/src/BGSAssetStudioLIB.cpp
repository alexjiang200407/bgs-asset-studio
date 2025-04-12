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

DXGI_FORMAT GetDXGIFormatFromString(const std::string& str)
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

string DXGIFormatToString(DXGI_FORMAT format)
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
	struct TexAssetType
	{
		string      name_match_string;
		regex       name_match_regex;
		DXGI_FORMAT opaque_format;
		DXGI_FORMAT transparent_format;
	};

	using namespace nlohmann;
	void to_json(json& j, const TexAssetType& tex_asset_type)
	{
		j = json{ { "regex", tex_asset_type.name_match_string },
			      { "dxgi-format-opaque", DXGIFormatToString(tex_asset_type.opaque_format) },
			      { "dxgi-format-transparent",
			        DXGIFormatToString(tex_asset_type.transparent_format) } };
	}

	void from_json(const json& j, TexAssetType& tex_asset_type)
	{
		j.at("regex").get_to(tex_asset_type.name_match_string);

		tex_asset_type.name_match_regex = regex(tex_asset_type.name_match_string);

		string buf;
		j.at("dxgi-format-opaque").get_to(buf);

		tex_asset_type.opaque_format = GetDXGIFormatFromString(buf);

		j.at("dxgi-format-transparent").get_to(buf);
		tex_asset_type.transparent_format = GetDXGIFormatFromString(buf);
	}
}
using namespace nlohmann;
class AssetBuilder
{
	stack<fs::path>                     path_stack;
	list<map<string, ns::TexAssetType>> tex_asset_stack;
	list<vector<string>>                order;

public:
	void push(const fs::path& p)
	{
		ifstream ifs(p);
		json     js        = json::parse(ifs);
		json     tex_types = js.at("tex-types");

		map<string, ns::TexAssetType> m;
		vector<string>                tex_order;

		if (tex_types.is_array())
		{
			for_e(field, tex_types)
			{
				ns::TexAssetType tex_asset;
				string           name = field.at("name");
				tex_order.push_back(name);
				m.insert(make_pair(name, field.get_to(tex_asset)));
			}
		}

		path_stack.push(p);
		order.push_back(tex_order);
		tex_asset_stack.push_back(m);
	};
	void            pop() { path_stack.pop(); };
	const fs::path& top() { return path_stack.top(); };
	const string   asset_type(const fs::path& p)
	{
		auto current_level = tex_asset_stack.rbegin();
		auto current_order = order.rbegin();
		while (current_level != tex_asset_stack.rend())
		{
			for_e(label, *current_order)
			{ 
				if (regex_match(p.filename().string(), current_level->at(label).name_match_regex))
				{
					return label;
				}
			}

			current_level++;
			current_order++;
		}
		return "not found";
	};
	bool empty() { return path_stack.size() == 0; };
};

void visit_directory(const fs::path& root)
{
	try
	{
		AssetBuilder builder;

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