#pragma once

#include "commands/core/CommandQueryRequest.h"
#include <vector>

// メインウインドウからコマンド一覧の問い合わせを行うためのリクエストクラス
// 検索が完了したときにウインドウメッセージベースで通知を行う
class MainWindowCommandQueryRequest : public launcherapp::commands::core::CommandQueryRequest
{
	using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;

public:
	MainWindowCommandQueryRequest(const CString& keyword, HWND hwndNotify, UINT notifyMsg);
	~MainWindowCommandQueryRequest();

	CString GetCommandParameter() override;
	void NotifyQueryComplete(bool isCancelled, std::vector<launcherapp::core::Command*>* result) override;
	uint32_t AddRef() override;
	uint32_t Release() override;

private:
	uint32_t mRefCount{1};
	CString mKeyword;
	HWND mHwnd{nullptr};
	UINT mMsg{0};
};

