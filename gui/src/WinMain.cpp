#include "BGSAssetStudioLIB.h"
#include <Windows.h>

#ifdef MEMORY_LEAK_CHECK
#	define _CRTDBG_MAP_ALLOC
#	include <crtdbg.h>
#endif

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
#ifdef MEMORY_LEAK_CHECK
	_CrtMemState sOld;
	_CrtMemState sNew;
	_CrtMemState sDiff;
	_CrtMemCheckpoint(&sOld);
#endif  // MEMORY_LEAK_CHECK

	MessageBoxA(NULL, foo(), "Info", MB_OK | MB_ICONINFORMATION);

#ifdef MEMORY_LEAK_CHECK
	_CrtMemCheckpoint(&sNew);
	if (_CrtMemDifference(&sDiff, &sOld, &sNew))
	{
		OutputDebugString(L"-----------_CrtMemDumpStatistics ---------");
		_CrtMemDumpStatistics(&sDiff);
		OutputDebugString(L"-----------_CrtMemDumpAllObjectsSince ---------");
		_CrtMemDumpAllObjectsSince(&sOld);
		OutputDebugString(L"-----------_CrtDumpMemoryLeaks ---------");
		_CrtDumpMemoryLeaks();
	}
#endif  // MEMORY_LEAK_CHECK

	return 0;
}