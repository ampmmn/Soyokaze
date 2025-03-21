#include "pch.h"
#include "framework.h"
#include "MMCCommand.h"
#include "commands/common/SubProcess.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace mmc {

struct MMCCommand::PImpl
{
	MMCSnapin mSnapin;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(MMCCommand)

MMCCommand::MMCCommand(const MMCSnapin& snapin) : 
	AdhocCommandBase(snapin.mDisplayName, snapin.mDescription),
	in(std::make_unique<PImpl>())
{
	in->mSnapin = snapin;
}

MMCCommand::~MMCCommand()
{
}

CString MMCCommand::GetGuideString()
{
	return _T("Enter:実行");
}

CString MMCCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE(_T("MMCスナップイン"));
	return TEXT_TYPE;
}

BOOL MMCCommand::Execute(Parameter* param)
{
	CString paramStr;
	paramStr.Format(_T("/c start \"\" \"%s\""), (LPCTSTR)in->mSnapin.mMscFilePath);

	SubProcess::ProcessPtr process;

	SubProcess exec(param);
	exec.SetShowType(SW_HIDE);
	if (exec.Run(_T("cmd.exe"), paramStr, process) == false) {
		mErrMsg = process->GetErrorMessage();
		return FALSE;
	}

	return TRUE;
}

HICON MMCCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconResource(in->mSnapin.mIconFilePath, in->mSnapin.mIconIndex);
}

launcherapp::core::Command*
MMCCommand::Clone()
{
	return new MMCCommand(in->mSnapin);
}

// メニューの項目数を取得する
int MMCCommand::GetMenuItemCount()
{
	return 1;
}

// メニューの表示名を取得する
bool MMCCommand::GetMenuItemName(int index, LPCWSTR* displayNamePtr)
{
	if (index == 0) {
		static LPCWSTR name = L"開く(&O)";
		*displayNamePtr= name;
		return true;
	}
	return false;
}

// メニュー選択時の処理を実行する
bool MMCCommand::SelectMenuItem(int index, launcherapp::core::CommandParameter* param)
{
	if (index == 0) {
		return Execute(param) != FALSE;
	}
	return false;
}

bool MMCCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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


} // end of namespace mmc
} // end of namespace commands
} // end of namespace launcherapp

