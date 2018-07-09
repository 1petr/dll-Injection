#include <windows.h>

DWORD APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        MessageBox(NULL, "Dll injected!", "All right!", MB_OK);
		break;
	}
	return true;
}
