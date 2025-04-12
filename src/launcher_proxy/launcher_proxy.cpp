// launcher_proxy.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する

#include <windows.h>
#include <string>
#include "Restart.h"
#include "NormalPriviledgeAgent.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (__argc == 1) {
		return 1;
	}
	std::wstring command_name(__wargv[1]);
	if (command_name == L"restart") {
		return RestartApp();
	}
	else if (command_name == L"run-normal-priviledge-agent") {
		NormalPriviledgeAgent agent;
		return agent.Run(hInstance);
	}

	// Unknown command
	return 1;
}
