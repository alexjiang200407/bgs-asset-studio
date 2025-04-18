#include "bgs_asset_studio.h"
#include "asset_builder.h"
#include "asset_registry_impl.h"
#include <concurrent_queue.h>

using namespace std;
namespace fs = std::filesystem;
using namespace concurrency;

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

void visit_directory(
	const fs::path&                        root,
	asset_builder&                         builder,
	concurrent_queue<asset_builder::task>& tasks)
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
				visit_directory(entry.path(), builder, tasks);

			if (!entry.is_directory())
			{
				spdlog::trace(
					"using {} for {}",
					builder.empty() ? "default" : (char*)builder.preset_name().data(),
					entry.path().string());

				auto build_task = builder.build(entry.path());
				if (build_task)
					tasks.push(build_task);
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

void handle_build_tasks(
	concurrent_queue<asset_builder::task>& tasks,
	const unique_ptr<asset_registry_impl>& assets,
	size_t                                 num_threads)
{
	std::vector<std::thread> threads;

	auto worker_func = [&]() {
		asset_builder::task task;
		while (true)
		{
			if (tasks.try_pop(task))
			{
				assets->insert(task());
			}
			else
			{
				spdlog::info("Finished");
				break;
			}
		}
	};

	for (size_t i = 0; i < num_threads; i++)
	{
		threads.emplace_back(worker_func);
	}

	for (auto& t : threads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}
}

LIBRARY_API asset_registry_handle
	register_assets(const fs::path& dir, const fs::path& preset_path, size_t num_threads)
{
	asset_builder                         builder;
	unique_ptr<asset_registry_impl>       assets = make_unique<asset_registry_impl>();
	concurrent_queue<asset_builder::task> build_tasks;

	try
	{
		builder.push(preset_path);
	}
	catch (const asset_builder::exception& err)
	{
		spdlog::error("Could not add preset path: {}", err.what());
	}

	visit_directory(dir, builder, build_tasks);
	handle_build_tasks(build_tasks, assets, num_threads);

	return move(assets);
}

LIBRARY_API asset_registry_handle register_assets(const fs::path& dir, size_t num_threads)
{
	unique_ptr<asset_registry_impl>       assets = make_unique<asset_registry_impl>();
	concurrent_queue<asset_builder::task> build_tasks;

	spdlog::warn("No default preset was used!");
	asset_builder builder;
	visit_directory(dir, builder, build_tasks);
	handle_build_tasks(build_tasks, assets, num_threads);

	return move(assets);
}
