#include "pch.h"
#include "framework.h"
#include "SpecialFolderFileCommand.h"
#include "commands/common/SubProcess.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/core/ActionParameter.h"
#include "icon/IconLoader.h"
#include "setting/AppPreference.h"
#include "utility/Path.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using NamedParameter = launcherapp::actions::core::NamedParameter;

namespace launcherapp {
namespace commands {
namespace specialfolderfiles {


struct SpecialFolderFileCommand::PImpl
{
	ITEM mItem;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(SpecialFolderFileCommand)

SpecialFolderFileCommand::SpecialFolderFileCommand(const ITEM& item) : 
	AdhocCommandBase(item.mName, item.mDescription),
	in(std::make_unique<PImpl>())
{
	in->mItem = item;
	if (item.mDescription.IsEmpty()) {
		this->mDescription = item.mFullPath;
	}
}

SpecialFolderFileCommand::~SpecialFolderFileCommand()
{
}

CString SpecialFolderFileCommand::GetGuideString()
{
	return _T("⏎:開く C-⏎:フォルダを開く");
}


CString SpecialFolderFileCommand::GetTypeDisplayName()
{
	return TypeDisplayName((int)in->mItem.mType);
}

BOOL SpecialFolderFileCommand::Execute(Parameter* param)
{
	SubProcess::ProcessPtr process;

	SubProcess exec(param);
	if (exec.Run(in->mItem.mFullPath, process) == false) {
		this->mErrMsg = process->GetErrorMessage();
		return FALSE;
	}

	return TRUE;
}

HICON SpecialFolderFileCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mItem.mFullPath);
}

launcherapp::core::Command*
SpecialFolderFileCommand::Clone()
{
	return new SpecialFolderFileCommand(in->mItem);
}

// メニューの項目数を取得する
int SpecialFolderFileCommand::GetMenuItemCount()
{
	return in->mItem.mType == TYPE_RECENT ? 3 : 2;
}

// メニューの表示名を取得する
bool SpecialFolderFileCommand::GetMenuItemName(int index, LPCWSTR* displayNamePtr)
{
	if (index == 0) {
		static LPCWSTR name = L"実行(&E)";
		*displayNamePtr= name;
		return true;
	}
	else if (index == 1) {
		static LPCWSTR name = L"パスを開く(&O)";
		*displayNamePtr= name;
		return true;
	}
	else if (index == 2) {
		static LPCWSTR name = L"最近使ったファイルから削除する(&M)";
		*displayNamePtr= name;
		return true;
	}
	return false;
}

// メニュー選択時の処理を実行する
bool SpecialFolderFileCommand::SelectMenuItem(int index, Parameter* param)
{
	if (index < 0 || 2 < index) {
		return false;
	}

	if (index == 0) {
		return Execute(param) != FALSE;
	}

	RefPtr<NamedParameter> namedParam;
	if (param->QueryInterface(IFID_COMMANDNAMEDPARAMETER, (void**)&namedParam) == false) {
		return false;
	}

	if (index == 1) {
		// パスを開くため、疑似的にCtrl押下で実行したことにする
		namedParam->SetNamedParamBool(_T("CtrlKeyPressed"), true);
		return Execute(param) != FALSE;
	}
	else  {
		// 削除する管理者権限で実行するため、疑似的にCtrl-Shift押下で実行したことにする
		if (in->mItem.mType != TYPE_RECENT) {
			// 削除できるのは履歴のみ
			return true;
		}

		if (Path::FileExists(in->mItem.mLinkPath) == FALSE) {
			return true;
		}
		// ショートカットを削除
		DeleteFile(in->mItem.mLinkPath);
		return true;
	}
}

bool SpecialFolderFileCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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

CString SpecialFolderFileCommand::TypeDisplayName(int type)
{
	if (type == TYPE_RECENT) {
		static CString TEXT_TYPE_RECENT((LPCTSTR)IDS_COMMAND_RECENTFILES);
		return TEXT_TYPE_RECENT;
	}
	else { // if (type == TYPE_STARTMENU)
		static CString TEXT_TYPE_STARTMENU((LPCTSTR)IDS_COMMAND_STARTMENU);
		return TEXT_TYPE_STARTMENU;
	}
}

} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace launcherapp

