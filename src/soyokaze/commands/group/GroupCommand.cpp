#include "pch.h"
#include "GroupCommand.h"
#include "core/IFIDDefine.h"
#include "commands/group/CommandParam.h"
#include "commands/group/GroupCommandEditor.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/core/CommandRepository.h"
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
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;


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
	BOOL Execute(Parameter* param, int round);

	CommandParam mParam;
	CString mErrMsg;
};


BOOL GroupCommand::PImpl::Execute(Parameter* param, int round)
{
	auto cmdRepo = CommandRepository::GetInstance();
	ASSERT(cmdRepo);

	CString cmdName = mParam.mName;

	// 初回に(必要に応じて)実行確認を行う
	auto namedParam = GetCommandNamedParameter(param);
	bool isConfirmed = namedParam->GetNamedParamBool(_T("CONFIRMED"));
	if (round == 0 && isConfirmed == false) {

		if (mParam.mIsConfirm) {
			CString msg;
			msg.Format(IDS_CONFIRMRUNGROUP, (LPCTSTR)cmdName);

			SharedHwnd sharedHwnd;

			CString caption((LPCTSTR)IDS_TITLE_CONFIRMRUNGROUP);
			int n = MessageBox(sharedHwnd.GetHwnd(), msg, caption, MB_YESNO);
			if (n != IDYES) {
				throw Exception();
			}

			// ひとたび確認を実施した場合は下位のグループで再度確認を実施しない。
			// (後でマークをつける)
			isConfirmed = true;
		}
	}


	CString parents;
	GetNamedParamString(param, _T("PARENTS"), parents);

	// 循環参照チェック
	int depth = 0;
	int n = 0;
	CString token = parents.Tokenize(_T("/"), n);
	while(token.IsEmpty() == FALSE) {

		if (depth >= 8) {
			// 深さは8まで
			return FALSE;
		}
		if (token == cmdName) {
			// 呼び出し元に自分自身がいる(循環参照)
			return FALSE;
		}
		token = parents.Tokenize(_T("/"), n);
		depth++;
	}

	// 呼び出し元に自分自身を追加
	if (parents.IsEmpty() == FALSE) {
		parents += _T("/");
	}
	parents += cmdName;

	for (auto& item : mParam.mItems) {

		RefPtr<launcherapp::core::Command> command(cmdRepo->QueryAsWholeMatch(item.mItemName, false));
		if (command == nullptr) {
			continue;
		}

		RefPtr<CommandParameterBuilder> paramSub(CommandParameterBuilder::Create(), false);
		if (mParam.mIsPassParam) {
			paramSub->SetParameterString(param->GetParameterString());
		}

		if (item.mIsWait) {
			paramSub->SetNamedParamBool(_T("WAIT"), true);
		}
		if (isConfirmed) {
			paramSub->SetNamedParamBool(_T("CONFIRMED"), true);
		}
		paramSub->SetNamedParamString(_T("PARENTS"), parents);

		command->Execute(paramSub);
	}

	return TRUE;
}

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

BOOL GroupCommand::Execute(Parameter* param)
{
	auto namedParam = launcherapp::commands::common::GetCommandNamedParameter(param);
	if (param->HasParameter() && namedParam->GetNamedParamBool(_T("RunAsHistory")) == false) {
		ExecuteHistory::GetInstance()->Add(_T("history"), param->GetWholeString());
	}

	try {
		int nRepeats = in->mParam.mIsRepeat ? in->mParam.mRepeats : 1;
		for (int round = 0; round < nRepeats; ++round) {
			in->Execute(param, round);
		}
		return TRUE;
	}
	catch(GroupCommand::Exception&) {
		return TRUE;
	}
}

CString GroupCommand::GetErrorString()
{
	return in->mErrMsg;
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
	clonedObj->in->mErrMsg = in->mErrMsg;

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

} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

