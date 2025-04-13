#include "BGSAssetStudioLIB.h"
#include "arg_actions.h"
#include <argparse/argparse.hpp>

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

		program.add_argument("-h", "--help").help("Show help message").flag();

		program.add_argument("--dryrun")
			.flag()
			.help("Run without making any changes (for testing)");

		program.add_argument("--max-dds-size")
			.action(argparse::texture_sz("max-dds-size"))
			.default_value(0)
			.metavar("[0-16384]")
			.help("Maximum dds texture size. Set to 0 to skip downscaling");

		program.add_argument("--min-dds-size")
			.action(argparse::texture_sz("min-dds-size"))
			.default_value(0)
			.metavar("[0-16384]")
			.help("Minimum texture size. Set to 0 to skip upscaling");

		program.add_argument("--threads")
			.action(argparse::range("threads", 0, 64))
			.default_value(0)
			.metavar("[0-64]")
			.help("Number of threads to use, 0 to automatically calculate");

		program.add_argument("--no-dds-compression")
			.default_value(false)
			.flag()
			.help("Don't compress DDS textures");

		program.add_argument("--preset")
			.default_value(std::filesystem::path())
			.action(argparse::filepath)
			.help("Input preset to use");

		program.add_argument("directory")
			.action(argparse::filepath)
			.required()
			.help("Input directory to process directory for");

		program.parse_args(argc, argv);

		auto directory          = program.get<filesystem::path>("directory");
		auto preset             = program.get<filesystem::path>("--preset");
		auto preset_is_used     = program.is_used("--preset");
		auto max_dds_size       = program.get<int>("--max-dds-size");
		auto min_dds_size       = program.get<int>("--min-dds-size");
		auto threads            = program.get<int>("--threads");
		auto no_dds_compression = program.get<bool>("--no-dds-compression");
		auto dryrun             = program.get<bool>("--dryrun");

		if (preset_is_used)
			register_assets(directory, preset);
		else
			register_assets(directory);

		dbg(foo());
		dbg(max_dds_size);
		dbg(preset_is_used);
		dbg(min_dds_size);
		dbg(threads);
		dbg(no_dds_compression);
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
