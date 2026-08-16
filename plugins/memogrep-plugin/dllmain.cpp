// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "pch.h"
#include <nlohmann/json.hpp>
#include <soyokaze/PluginExportTable.h>

HMODULE g_hModule = nullptr;

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			g_hModule = hModule;
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

