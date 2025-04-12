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
#ifdef MEMORY_LEAK_CHECK
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
	return ret;
}

int parse_args(int argc, char** argv)
{
	argparse::ArgumentParser program("bgs-asset-studio-cli");

	try
	{
		program.set_usage_max_line_width(80);
		program.set_usage_break_on_mutex();
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

		program.add_argument("--no-multi-threading")
			.default_value(false)
			.flag()
			.help("Enable multithreaded file processing (default: true)");

		program.add_argument("--no-dds-compression")
			.default_value(false)
			.flag()
			.help("Compress DDS textures (default: true)");

		program.add_argument("files")
			.remaining()
			.default_value(vector<filesystem::path>{})
			.action(argparse::filepath)
			.help("Input files to process");

		program.parse_args(argc, argv);

		auto files            = program.get<vector<filesystem::path>>("files");
		auto max_texture_size = program.get<int>("--max-dds-size");
		auto min_texture_size = program.get<int>("--min-dds-size");
		auto multi_threaded   = program.get<bool>("--no-multi-threading");
		auto compress_dds     = program.get<bool>("--no-dds-compression");
		auto dryrun           = program.get<bool>("--dryrun");

		dbg(foo());
		dbg(max_texture_size);
		dbg(min_texture_size);
		dbg(multi_threaded);
		dbg(compress_dds);
		dbg(dryrun);
		dbg(files);
	}
	catch (const exception& err)
	{
		cerr << err.what() << endl;
		cerr << program;
		return 1;
	}

	return 0;
}
