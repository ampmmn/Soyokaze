// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "pch.h"
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "PythonProxy.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


extern "C"
__declspec(dllexport) 
int
pythonproxy_Initialize()
{
	return 0;
}

extern "C"
__declspec(dllexport) 
int
pythonproxy_GetProxyObject(PythonProxyIF** proxy)
{
	static PythonProxy inst;
	*proxy = &inst;

	return 0;
}


