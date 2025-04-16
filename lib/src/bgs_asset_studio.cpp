#include "bgs_asset_studio.h"
#include "asset_builder.h"

using namespace std;
namespace fs = std::filesystem;

LIBRARY_API const char* foo() { return "Hello World!"; }

LIBRARY_API void init_log()
{
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto file_sink    = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log.txt", true);

	std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };
	auto log = std::make_shared<spdlog::logger>("global log"s, sinks.begin(), sinks.end());

	auto log_level = spdlog::level::info;

	log->set_level(log_level);
	log->flush_on(log_level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("\x1b[33m[%Y-%m-%d %H:%M:%S]\x1b[0m [%^%l%$] [thread %t] %v"s);
}

LIBRARY_API void stop_log()
{
	spdlog::info("Stopping log");

	spdlog::default_logger()->flush();
	spdlog::set_default_logger(nullptr);
	spdlog::drop_all();
	spdlog::shutdown();
}

void visit_directory(const fs::path& root, asset_builder& builder, set<asset_ptr>& assets)
{
	try
	{
		bool pop = false;

		if (fs::exists(root / "bgs-asset-studio-meta.json"))
		{
			try
			{
				spdlog::trace("Found new metadata file");
				builder.push(root / "bgs-asset-studio-meta.json");
				pop = true;
			}
			catch (const asset_builder::exception& err)
			{
				spdlog::error("Error: {}", err.what());
			}
		}

		for_e(entry, fs::directory_iterator(root))
		{
			if (entry.is_directory())
				visit_directory(entry.path(), builder, assets);

			if (!entry.is_directory())
			{
				spdlog::trace(
					"using {} for {}",
					builder.empty() ? "default" : (char*)builder.preset_name().data(),
					entry.path().string());
				spdlog::info(
					"{} is of type {}",
					entry.path().string(),
					(char*)builder.asset_type(entry.path()).data());

				auto asset_ptr = builder.build(entry.path());

				if (asset_ptr.get())
					assets.insert(move(asset_ptr));
			}
		}

		if (pop)
			builder.pop();
	}
	catch (const fs::filesystem_error& err)
	{
		spdlog::error(err.what());
	}
}

LIBRARY_API set<asset_ptr> register_assets(const fs::path& dir, const fs::path& preset_path)
{
	asset_builder  builder;
	set<asset_ptr> assets;

	try
	{
		builder.push(preset_path);
	}
	catch (const asset_builder::exception& err)
	{
		spdlog::error("Could not add preset path: {}", err.what());
	}

	visit_directory(dir, builder, assets);

	return assets;
}

LIBRARY_API set<asset_ptr> register_assets(const fs::path& dir)
{
	set<asset_ptr> assets;
	spdlog::warn("No default preset was used!");
	asset_builder builder;
	visit_directory(dir, builder, assets);

	return assets;
}