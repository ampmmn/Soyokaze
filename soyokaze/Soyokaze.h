
// Soyokaze.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CSoyokazeApp:
// このクラスの実装については、Soyokaze.cpp を参照してください
//

class CSoyokazeApp : public CWinApp
{
public:
	CSoyokazeApp();
	virtual ~CSoyokazeApp();

protected:
	HANDLE m_hMutexRun;
// オーバーライド
public:
	virtual BOOL InitInstance();

	BOOL InitFirstInstance();
	BOOL InitSecondInstance();

	// 先行するSoyokazeプロセスが存在するか?
	bool SoyokazeProcessExists();
	// 先行するプロセスがあればそちらを有効化する
	bool ActivateExistingProcess();

	bool SendCommandString(const CString& commandStr);
	bool RegisterPath(const CString& pathStr);

// 実装
protected:

	DECLARE_MESSAGE_MAP()
};

extern CSoyokazeApp theApp;
