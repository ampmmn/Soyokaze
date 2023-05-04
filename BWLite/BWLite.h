
// BWLite.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CBWLiteApp:
// このクラスの実装については、BWLite.cpp を参照してください
//

class CBWLiteApp : public CWinApp
{
public:
	CBWLiteApp();
	virtual ~CBWLiteApp();

protected:
	HANDLE m_hMutexRun;
// オーバーライド
public:
	virtual BOOL InitInstance();

	// 先行するBWLiteプロセスが存在するか?
	bool BWLiteProcessExists();
	// 先行するプロセスがあればそちらを有効化する
	bool ActivateExistingProcess();

	bool SendCommandString(const CString& commandStr);

// 実装
protected:

	DECLARE_MESSAGE_MAP()
};

extern CBWLiteApp theApp;
