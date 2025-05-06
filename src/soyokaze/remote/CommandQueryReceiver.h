#pragma once

#include "commands/core/CommandIF.h"
#include <memory>
#include <vector>

namespace launcherapp { namespace remote {

/**
  検索結果を受け取るためのクラス
*/
class QueryReceiver
{
	struct QUERY_RESULT;
public:
	QueryReceiver();
	~QueryReceiver();

	void QuerySync(const std::wstring& query, std::vector<launcherapp::core::Command*>& commands);

private:
	bool CreateReceiverWindow();
	static LRESULT CALLBACK OnWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	HWND mHwnd{nullptr};
};


}} // end of namespace launcherapp::remote

