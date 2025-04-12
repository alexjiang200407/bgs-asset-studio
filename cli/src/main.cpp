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

		program.add_argument("--threads")
			.action(argparse::range("threads", 0, 64))
			.default_value(0)
			.metavar("[0-64]")
			.help("Number of threads to use, 0 to automatically calculate");

		program.add_argument("--no-dds-compression")
			.default_value(false)
			.flag()
			.help("Don't compress DDS textures");

		program.add_argument("files")
			.remaining()
			.default_value(vector<filesystem::path>{})
			.action(argparse::filepath)
			.help("Input files to process");

		program.add_argument("--normalmap-regex")
			.default_value(regex("(_msn|_n).dds$"))
			.action(argparse::regex_action("normalmap-regex"))
			.help("regex to match for normal map textures");

		program.add_argument("--glowmap-regex")
			.default_value(regex("_g.dds$"))
			.action(argparse::regex_action("glowmap-regex"))
			.help("regex to match for glow maps textures");

		program.add_argument("--subsurface-regex")
			.default_value(regex("_sk.dds$"))
			.action(argparse::regex_action("subsurface-regex"))
			.help("regex to match for subsurface textures");

		program.add_argument("--metallic-regex")
			.default_value(regex("_m.dds$"))
			.action(argparse::regex_action("metallic-regex"))
			.help("regex to match for metallic textures");

		program.add_argument("--diffuse-regex")
			.default_value(regex(".dds$"))
			.action(argparse::regex_action("diffuse-regex"))
			.help("regex to match for diffuse textures");

		program.add_argument("--texture-process-order")
			.default_value(
				vector<string>{ "normalmap", "glowmap", "subsurface", "metallic", "diffuse" })
			.help(
				"order in which the textures are processed. Default processes normalmaps first and "
				"diffuse last.");

		program.parse_args(argc, argv);

		auto files              = program.get<vector<filesystem::path>>("files");
		auto max_dds_size       = program.get<int>("--max-dds-size");
		auto min_dds_size       = program.get<int>("--min-dds-size");
		auto threads            = program.get<int>("--threads");
		auto no_dds_compression = program.get<bool>("--no-dds-compression");
		auto dryrun             = program.get<bool>("--dryrun");
		auto normalmap_regex    = program.get<regex>("--normalmap-regex");
		auto glowmap_regex      = program.get<regex>("--glowmap-regex");
		auto subsurface_regex   = program.get<regex>("--subsurface-regex");
		auto metallic_regex     = program.get<regex>("--metallic-regex");
		auto diffuse_regex      = program.get<regex>("--diffuse-regex");
		auto order              = program.get<vector<string>>("--texture-process-order");

		if (max_dds_size < min_dds_size)
			throw runtime_error("max-dds-size must be greater or equal to min-dds-size");

		dbg(foo());
		dbg(max_dds_size);
		dbg(min_dds_size);
		dbg(threads);
		dbg(no_dds_compression);
		dbg(dryrun);
		dbg(files);
		dbg(order);
	}
	catch (const exception& err)
	{
		cerr << err.what() << endl;
		return 1;
	}

	return 0;
}
