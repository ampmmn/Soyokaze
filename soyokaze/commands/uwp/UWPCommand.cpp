#include "pch.h"
#include "framework.h"
#include "UWPCommand.h"
#include "commands/common/SubProcess.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace uwp {


struct UWPCommand::PImpl
{
	ItemPtr mItem;
};


UWPCommand::UWPCommand(ItemPtr& item) : 
	AdhocCommandBase(item->mName, item->mName),
	in(std::make_unique<PImpl>())
{
	in->mItem = item;
}

UWPCommand::~UWPCommand()
{
}

CString UWPCommand::GetGuideString()
{
	return _T("Enter:実行");
}

CString UWPCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_APP);
	return TEXT_TYPE;
}

BOOL UWPCommand::Execute(Parameter* param)
{
	CString paramStr;
	if (in->mItem->mIsUWP) {
		paramStr.Format(_T("/c start shell:AppsFolder\\%s:"), (LPCTSTR)in->mItem->mAppID);
	}
	else {
		paramStr.Format(_T("/c start \"\" \"%s\""), (LPCTSTR)in->mItem->mAppID);
	}

	SubProcess::ProcessPtr process;

	SubProcess exec(param);
	exec.SetShowType(SW_HIDE);
	if (exec.Run(_T("cmd.exe"), paramStr, process) == false) {
		mErrMsg = process->GetErrorMessage();
		return FALSE;
	}

	return TRUE;
}

HICON UWPCommand::GetIcon()
{
	return in->mItem->mIcon;
}

launcherapp::core::Command*
UWPCommand::Clone()
{
	return new UWPCommand(in->mItem);
}

} // end of namespace uwp
} // end of namespace commands
} // end of namespace launcherapp

