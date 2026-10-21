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
		// パイプ名は親プロセスから引き渡される
		// (固定名では他のプロセスから接続され、悪用される可能性があるため)
		return agent.Run(GetModuleHandle(nullptr), argc > 2 ? argv[2] : nullptr);
	}

	// Unknown command
	fwprintf(stderr, L"Unknown command: %s\n", command_name.c_str());
	return 1;
}
