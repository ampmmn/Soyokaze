#include "pch.h"
#include "framework.h"
#include "UWPCommand.h"
#include "commands/common/SubProcess.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace soyokaze::commands::common;

namespace soyokaze {
namespace commands {
namespace uwp {


struct UWPCommand::PImpl
{
	ITEM mItem;
};


UWPCommand::UWPCommand(const ITEM& item) : 
	AdhocCommandBase(item.mName, item.mDescription),
	in(std::make_unique<PImpl>())
{
	in->mItem = item;
}

UWPCommand::~UWPCommand()
{
}


CString UWPCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_UWP);
	return TEXT_TYPE;
}

BOOL UWPCommand::Execute(const Parameter& param)
{
	CString paramStr;
	paramStr.Format(_T("/c start shell:AppsFolder\\%s:"), in->mItem.mAppID);

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
	return in->mItem.mIcon;
}

soyokaze::core::Command*
UWPCommand::Clone()
{
	return new UWPCommand(in->mItem);
}

} // end of namespace uwp
} // end of namespace commands
} // end of namespace soyokaze

