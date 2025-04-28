#include "pch.h"
#include "framework.h"
#include "MSSettingsCommand.h"
#include "commands/common/SubProcess.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace mssettings {

struct MSSettingsCommand::PImpl
{
	CString mScheme;
	CString mCategory;
	CString mPageTitle;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(MSSettingsCommand)

MSSettingsCommand::MSSettingsCommand(
	const CString& scheme,
	const CString& category,
	const CString& pageTitle
) : AdhocCommandBase(_T(""), _T("")),
	in(std::make_unique<PImpl>())
{

	this->mName.Format(_T("%s > %s"), (LPCTSTR)category, (LPCTSTR)pageTitle);
	this->mDescription= this->mName;

	in->mScheme = scheme;
	in->mCategory = category;
	in->mPageTitle = pageTitle;
}

MSSettingsCommand::~MSSettingsCommand()
{
}

CString MSSettingsCommand::GetGuideString()
{
	return _T("⏎:開く");
}

CString MSSettingsCommand::GetTypeDisplayName()
{
	return _T("Windowsの設定");
}

BOOL MSSettingsCommand::Execute(Parameter* param)
{
	SubProcess exec(param);
	SubProcess::ProcessPtr process;
	return exec.Run(in->mScheme, process);
}

HICON MSSettingsCommand::GetIcon()
{
	return IconLoader::Get()->GetShell32Icon(-16826);
}

launcherapp::core::Command*
MSSettingsCommand::Clone()
{
	return new MSSettingsCommand(in->mScheme, in->mCategory, in->mPageTitle);
}

// メニューの項目数を取得する
int MSSettingsCommand::GetMenuItemCount()
{
	return 1;
}

// メニューの表示名を取得する
bool MSSettingsCommand::GetMenuItemName(int index, LPCWSTR* displayNamePtr)
{
	if (index == 0) {
		static LPCWSTR name = L"開く(&O)";
		*displayNamePtr= name;
		return true;
	}
	return false;
}

// メニュー選択時の処理を実行する
bool MSSettingsCommand::SelectMenuItem(int index, launcherapp::core::CommandParameter* param)
{
	if (index == 0) {
		return Execute(param) != FALSE;
	}
	return false;
}

bool MSSettingsCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_CONTEXTMENUSOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ContextMenuSource*)this;
		return true;
	}
	return false;
}


} // end of namespace controlpanel
} // end of namespace commands
} // end of namespace launcherapp

