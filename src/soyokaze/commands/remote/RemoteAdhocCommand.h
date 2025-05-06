#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp { namespace commands { namespace remote {

class RemoteClient;

class RemoteAdhocCommand :
	virtual	public launcherapp::commands::common::AdhocCommandBase
{
public:
	RemoteAdhocCommand(RemoteClient* client, LPCWSTR name, LPCWSTR description, LPCWSTR typeName,  LPCWSTR queryString, int index);
	virtual ~RemoteAdhocCommand();

	static RemoteAdhocCommand* CreateInstance(RemoteClient* client, LPCWSTR name, LPCWSTR description, LPCWSTR typeName, LPCWSTR queryString, int index);

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(RemoteAdhocCommand)
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::remote

