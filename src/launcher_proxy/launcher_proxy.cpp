// launcher_proxy.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する

#include <windows.h>
#include <locale.h>
#include <string>
#include "Restart.h"
#include "NormalPriviledgeAgent.h"

int wmain(int argc, wchar_t* argv[]) {

	_wsetlocale(LC_ALL, L"");

	if (argc == 1) {
		return 1;
	}
	std::wstring command_name(argv[1]);
	if (command_name == L"restart") {
		return RestartApp();
	}
	else if (command_name == L"run-normal-priviledge-agent") {
		NormalPriviledgeAgent agent;
		return agent.Run(GetModuleHandle(nullptr));
	}

	// Unknown command
	fwprintf(stderr, L"Unknown command: %s\n", command_name.c_str());
	return 1;
}
