#include "pch.h"
#include "CommandLineProcessor.h"
#include "app/StartupParam.h"
#include "SharedHwnd.h"

namespace launcherapp {

bool CommandLineProcessor::Run(int argc, TCHAR* argv[], SecondProcessProxyIF* proxy)
{
	StartupParam startupParam(argc, argv);

	bool hasRunCommand = false;
	CString value;

	if (startupParam.HasChangeDirectoryOption(value)) {
		proxy->ChangeDirectory(value);
	}

	while (startupParam.HasRunCommand(value)) {
		// -cオプションでコマンドが与えられた場合、既存プロセス側にコマンドを送り、終了する
		proxy->SendCommandString(value, false);
		hasRunCommand = true;
		startupParam.ShiftRunCommand();
	}

	if (hasRunCommand) {
		// -cオプションを処理したら、プログラムを終了する
		return false;
	}

	if (startupParam.HasPathToRegister(value)) {
		// 第一引数で存在するパスが指定された場合は登録画面を表示する
		proxy->RegisterPath(value);
		return false;
	}

	if (startupParam.HasHideOption()) {
		proxy->Hide();
		return false;
	}

	// プロセスをアクティブ化し、このプロセスは終了する
	proxy->Show();

	if (startupParam.HasPasteOption(value)) {
		// /pasteオプションでコマンドが与えられた場合、既存プロセス側にテキストを送る
		// 直前で実行したproxy.Show()により、入力欄のクリアが走るため、テキストを送る処理をあとに行っている
		spdlog::debug(_T("HasPaste value:{}"), (LPCTSTR)value);

		bool isPasteOnly = true;
		proxy->SendCommandString(value, isPasteOnly);
	}

	// 選択範囲を指定するオプションが指定されていたら範囲を送信する
	int startPos = -1, selLength = 0;
	if (startupParam.GetSelectRange(startPos, selLength)) {
		proxy->SendCaretRange(startPos, selLength);
	}

	return false;
}


}
