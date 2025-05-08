#include "arg_actions.h"
#include "bgs_asset_studio.h"
#include <argparse/argparse.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef MEMORY_LEAK_CHECK
#	define _CRTDBG_MAP_ALLOC
#	include <crtdbg.h>
#endif

using namespace std;

int parse_args(int argc, char** argv);

int main(int argc, char** argv)
{
	init_log();
#ifdef MEMORY_LEAK_CHECK
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & ~_CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);

	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);

	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

	_CrtMemState sOld;
	_CrtMemState sNew;
	_CrtMemState sDiff;
	_CrtMemCheckpoint(&sOld);
#endif  // MEMORY_LEAK_CHECK

	int ret = parse_args(argc, argv);

#ifdef MEMORY_LEAK_CHECK
	_CrtMemCheckpoint(&sNew);
	if (_CrtMemDifference(&sDiff, &sOld, &sNew))
	{
		printf("-----------_CrtMemDumpStatistics ---------");
		_CrtMemDumpStatistics(&sDiff);
		printf("-----------_CrtMemDumpAllObjectsSince ---------");
		_CrtMemDumpAllObjectsSince(&sOld);
		printf("-----------_CrtDumpMemoryLeaks ---------");
		_CrtDumpMemoryLeaks();
	}
#endif  // MEMORY_LEAK_CHECK

	stop_log();
	return ret;
}

int parse_args(int argc, char** argv)
{
	try
	{
		argparse::ArgumentParser program("bgs-asset-studio-cli");
		program.set_usage_max_line_width(80);
		program.set_usage_break_on_mutex();

		program.add_argument("--dryrun")
			.flag()
			.help("Run without making any changes (for testing)");

		program.add_argument("--max-dds-size")
			.action(argparse::texture_sz("max-dds-size"))
			.default_value(16384)
			.metavar("[1-16384]")
			.help("Maximum dds texture size. Empty to skip downscaling");

		program.add_argument("--threads")
			.action(argparse::range("threads", 1, 64))
			.default_value(max(1, int(std::thread::hardware_concurrency() - 2)))
			.metavar("[1-64]")
			.help("Number of threads to use, leave empty to automatically calculate");

		program.add_argument("--preset")
			.default_value(std::string())
			.help("Input game preset to use");

		program.add_argument("directory")
			.action(argparse::filepath)
			.required()
			.help("Input directory to process directory for");

		program.parse_args(argc, argv);

		auto directory      = program.get<filesystem::path>("directory");
		auto game_preset    = program.get<string>("--preset");
		auto preset_is_used = program.is_used("--preset");
		auto max_dds_size   = program.get<int>("--max-dds-size");
		auto threads        = program.get<int>("--threads");
		auto dryrun         = program.get<bool>("--dryrun");

		unique_ptr<asset_registry> assets;

		char buf[1024];

		if (preset_is_used && GetModuleFileName(NULL, buf, sizeof(buf)))
		{
			filesystem::path path = buf;
			path                  = path.parent_path();
			path                  = path / "presets" / (game_preset + ".json");
			assets                = register_assets(directory, path, threads, max_dds_size);
		}
		else
			assets = register_assets(directory, threads, max_dds_size);

		if (!dryrun)
			assets->process_all(threads);

		dbg(foo());
		dbg(max_dds_size);
		dbg(preset_is_used);
		dbg(threads);
		dbg(dryrun);
		dbg(directory);
	}
	catch (const exception& err)
	{
		cerr << err.what() << endl;
		return 1;
	}

	return 0;
}
