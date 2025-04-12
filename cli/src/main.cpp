#include "BGSAssetStudioLIB.h"
#include <stdio.h>

#ifdef MEMORY_LEAK_CHECK
#	define _CRTDBG_MAP_ALLOC
#	include <crtdbg.h>
#endif

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

	printf("%s\n", foo());

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
	return 0;
}