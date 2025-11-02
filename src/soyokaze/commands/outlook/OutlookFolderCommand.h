#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp { namespace commands { namespace outlook {


class OutlookFolderCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase
{
public:
	OutlookFolderCommand(const CString& name, int itemCount, const CString& entryID);
	virtual ~OutlookFolderCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(OutlookFolderCommand)

public:
	static CString TypeDisplayName();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::outlook

