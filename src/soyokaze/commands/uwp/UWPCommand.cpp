#include "pch.h"
#include "framework.h"
#include "UWPCommand.h"
#include "actions/builtin/ExecuteAction.h"
#include "utility/Path.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;

namespace launcherapp {
namespace commands {
namespace uwp {


struct UWPCommand::PImpl
{
	ItemPtr mItem;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(UWPCommand)

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
	return _T("⏎:実行");
}

CString UWPCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool UWPCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	UNREFERENCED_PARAMETER(modifierFlags);

	Path cmdExePath(Path::SYSTEMDIR, _T("cmd.exe"));

	CString paramStr;
	if (in->mItem->mIsUWP) {
		paramStr.Format(_T("/c start shell:AppsFolder\\%s:"), (LPCTSTR)in->mItem->mAppID);
	}
	else {
		paramStr.Format(_T("/c start \"\" \"%s\""), (LPCTSTR)in->mItem->mAppID);
	}

	auto a = new ExecuteAction((LPCTSTR)cmdExePath, paramStr);
	a->SetShowType(SW_HIDE);
	*action = a;

	return true;
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

CString UWPCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_APP);
	return TEXT_TYPE;
}

} // end of namespace uwp
} // end of namespace commands
} // end of namespace launcherapp

