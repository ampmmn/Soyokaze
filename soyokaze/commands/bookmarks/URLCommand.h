#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace bookmarks {

class URLCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	enum Type {
		BOOKMARK,
		HISTORY,
	};
public:
	URLCommand(const CString& browserName, int type, const CString& name, const CString& url);
	virtual ~URLCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

