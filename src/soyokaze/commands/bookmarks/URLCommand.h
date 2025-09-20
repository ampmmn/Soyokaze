#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ExtraCandidateIF.h"
#include "commands/bookmarks/BookmarkItem.h"
#include "externaltool/webbrowser/BrowserEnvironment.h"
#include <memory>

namespace launcherapp { namespace commands { namespace bookmarks {

class URLCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ExtraCandidate
{
public:
	using BrowserEnvironment = launcherapp::externaltool::webbrowser::BrowserEnvironment;
public:
	URLCommand(const Bookmark& item, BrowserEnvironment* brwsEnv);
	virtual ~URLCommand();

	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ExtraCandidate
	CString GetSourceName() override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(URLCommand)

public:
	static CString TypeDisplayName(LPCTSTR productName);
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::bookmarks
