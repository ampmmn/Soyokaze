#pragma once

#include "commands/core/CommandQueryRequest.h"
#include <vector>

// $B%a%$%s%&%$%s%I%&$+$i%3%^%s%I0lMw$NLd$$9g$o$;$r9T$&$?$a$N%j%/%(%9%H%/%i%9(B
// $B8!:w$,40N;$7$?$H$-$K%&%$%s%I%&%a%C%;!<%8%Y!<%9$GDLCN$r9T$&(B
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

