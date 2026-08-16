#include "pch.h"

HMODULE g_hModule = nullptr;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    UNREFERENCED_PARAMETER(lpReserved);
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        g_hModule = hModule;
    }
    return TRUE;
}

