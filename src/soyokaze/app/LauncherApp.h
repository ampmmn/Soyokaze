
// LauncherApp.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// LauncherApp:
// このクラスの実装については、LauncherApp.cpp を参照してください
//

class TaskTray;

class LauncherApp : public CWinApp
{
public:
	LauncherApp();
	virtual ~LauncherApp();

protected:
	std::unique_ptr<TaskTray> mTaskTray;

// オーバーライド
public:
	virtual BOOL InitInstance();

	BOOL InitFirstInstance();
	BOOL InitSecondInstance();

	// バルーンメッセージを表示
	bool PopupMessage(const CString& message);

// 実装
protected:

	DECLARE_MESSAGE_MAP()
};

extern LauncherApp theApp;

