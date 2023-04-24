
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

	// 既存のプロセスを有効にする
	void ActivateExistingProcess();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CBWLiteApp theApp;
