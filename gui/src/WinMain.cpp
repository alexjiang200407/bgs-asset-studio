#include <Windows.h>
#include "BGSAssetStudioLIB.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	MessageBox(NULL, foo(), "Info", MB_OK | MB_ICONINFORMATION);
	return 0;
}