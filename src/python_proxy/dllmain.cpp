// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "pch.h"
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "PythonProxy.h"
#include "ProxyWindow.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
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
	return ProxyWindow::GetInstance()->Initialize();
}

static PythonProxy* GetProxyObj()
{
    static PythonProxy inst;
    return &inst;
}

extern "C"
__declspec(dllexport) 
int
pythonproxy_GetProxyObject(PythonProxyIF** proxy)
{
	*proxy = GetProxyObj();
	return 0;
}

extern "C"
__declspec(dllexport)
int
pythonproxy_Finalize()
{
	ProxyWindow::GetInstance()->Abort();
	GetProxyObj()->Finalize();
	ProxyWindow::GetInstance()->Finalize();
	return 0;
}


