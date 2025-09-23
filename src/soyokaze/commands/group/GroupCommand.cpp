#include "pch.h"
#include "GroupCommand.h"
#include "core/IFIDDefine.h"
#include "commands/group/CommandParam.h"
#include "commands/group/GroupCommandEditor.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/core/CommandRepository.h"
#include "actions/core/ActionParameter.h"
#include "actions/builtin/GroupAction.h"
#include "actions/builtin/RunCommandAction.h"
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace group {

using namespace launcherapp::commands::common;

using CommandRepository = launcherapp::core::CommandRepository;
using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;
using GroupAction = launcherapp::actions::builtin::GroupAction;
using RunCommandAction = launcherapp::actions::builtin::RunCommandAction;

// もしグループ実行を止めるような機構をいれる場合は
// 実行処理の中でこれをthrowする
class GroupCommand::Exception
{
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct GroupCommand::PImpl
{
	CommandParam mParam;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString GroupCommand::GetType() { return _T("Group"); }

CString GroupCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_GROUPCOMMAND);
	return TEXT_TYPE;
}

GroupCommand::GroupCommand() : in(std::make_unique<PImpl>())
{
}

GroupCommand::~GroupCommand()
{
}

CString GroupCommand::GetName()
{
	return in->mParam.mName;
}


CString GroupCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString GroupCommand::GetGuideString()
{
	return _T("⏎:開く");
}

CString GroupCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool GroupCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	auto groupAction = new GroupAction(in->mParam.mName, modifierFlags);
	groupAction->EnableConfirm(in->mParam.mIsConfirm);
	groupAction->SetRepeats(in->mParam.mIsRepeat ? in->mParam.mRepeats : 1);
	groupAction->EnablePassParam(in->mParam.mIsPassParam);

	for (auto& item : in->mParam.mItems) {
		RefPtr<RunCommandAction> subAction(new RunCommandAction(in->mParam.mName, item.mItemName, modifierFlags));
		subAction->EnableWait(item.mIsWait);
		groupAction->AddAction(subAction.get());
	}

	*action = groupAction;

	return true;
}

HICON GroupCommand::GetIcon()
{
	return IconLoader::Get()->LoadGroupIcon();
}


void GroupCommand::SetParam(const CommandParam& param)
{
	// 更新前に有効パラメータが存在し，かつ、自動実行を許可する場合は
	// 以前の名前で登録していた、履歴の除外ワードを解除する
	if (in->mParam.mName.IsEmpty() == FALSE && IsAllowAutoExecute()) {
		ExecuteHistory::GetInstance()->RemoveExcludeWord(in->mParam.mName);
	}

	// パラメータを上書き
	in->mParam = param;

	// 更新後に自動実行を許可する場合は履歴の除外ワードを登録する
	// (自動実行したいコマンド名が履歴に含まれると、自動実行を阻害することがあるため)
	if (in->mParam.mName.IsEmpty() == FALSE && IsAllowAutoExecute()) {
		ExecuteHistory::GetInstance()->AddExcludeWord(in->mParam.mName);
	}
}

void GroupCommand::AddItem(LPCTSTR itemName, bool isWait)
{
	GroupItem item;
	item.mItemName = itemName;
	item.mIsWait = isWait;

	in->mParam.mItems.push_back(item);
}

int GroupCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool GroupCommand::IsAllowAutoExecute()
{
	return in->mParam.mIsAllowAutoExecute;
}


bool GroupCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
GroupCommand::Clone()
{
	auto clonedObj = make_refptr<GroupCommand>();

	clonedObj->in->mParam = in->mParam;

	return clonedObj.release();
}

bool GroupCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);
	entry->Set(_T("Type"), GetType());

	return in->mParam.Save(entry);
}

bool GroupCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	if (GetType() != entry->Get(_T("Type"), _T(""))) {
		return false;
	}

	CommandParam param;
	if (param.Load(entry) == false) {
		return false;
	}

	SetParam(param);
	return true;
}

// コマンドを編集するためのダイアログを作成/取得する
bool GroupCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
{
	if (editor == nullptr) {
		return false;
	}

	auto cmdEditor = new CommandEditor(CWnd::FromHandle(parent));
	cmdEditor->SetParam(in->mParam);

	*editor = cmdEditor;
	return true;
}

// ダイアログ上での編集結果をコマンドに適用する
bool GroupCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_GROUPCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	SetParam(cmdEditor->GetParam());
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool GroupCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_GROUPCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<GroupCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

// コマンド新規作成ダイアログ
bool GroupCommand::NewDialog(Parameter* param)
{
	// グループ作成ダイアログを表示
	CString value;
	CommandParam paramTmp;

	if (GetNamedParamString(param, _T("COMMAND"), value)) {
		paramTmp.mName = value;
	}
	if (GetNamedParamString(param, _T("DESCRIPTION"), value)) {
		paramTmp.mDescription = value;
	}

	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	cmdEditor->SetParam(paramTmp);
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<GroupCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());

	return true;
}

} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

