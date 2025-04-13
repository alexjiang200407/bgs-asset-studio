#include "BGSAssetStudioLIB.h"
#include "asset_builder.h"
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