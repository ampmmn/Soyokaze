#include "pch.h"
#include "GroupCommand.h"
#include "commands/group/CommandParam.h"
#include "commands/group/GroupEditDialog.h"
#include "commands/common/ExecuteHistory.h"
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


using CommandRepository = launcherapp::core::CommandRepository;
using ExecuteHistory = launcherapp::commands::common::ExecuteHistory;

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
	BOOL Execute(const Parameter& param, int round);

	CommandParam mParam;
	CommandHotKeyAttribute mHotKeyAttr;

	CString mErrMsg;
};


BOOL GroupCommand::PImpl::Execute(const Parameter& param, int round)
{
	auto cmdRepo = CommandRepository::GetInstance();
	ASSERT(cmdRepo);

	CString cmdName = mParam.mName;

	// 初回に(必要に応じて)実行確認を行う
	bool isConfirmed = param.GetNamedParamBool(_T("CONFIRMED"));
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
	param.GetNamedParam(_T("PARENTS"), &parents);

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

		auto command = cmdRepo->QueryAsWholeMatch(item.mItemName, false);
		if (command == nullptr) {
			continue;
		}

		Parameter paramSub;
		if (mParam.mIsPassParam) {
			param.CopyParamTo(paramSub);
		}
		if (item.mIsWait) {
			paramSub.SetNamedParamBool(_T("WAIT"), true);
		}
		if (isConfirmed) {
			paramSub.SetNamedParamBool(_T("CONFIRMED"), true);
		}
		paramSub.SetNamedParamString(_T("PARENTS"), parents);

		command->Execute(paramSub);

		command->Release();
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString GroupCommand::GetType() { return _T("Group"); }

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
	return _T("Enter:開く");
}

CString GroupCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_GROUPCOMMAND);
	return TEXT_TYPE;
}

BOOL GroupCommand::Execute(const Parameter& param)
{
	if (param.GetParameterString().IsEmpty() == FALSE) {
		ExecuteHistory::GetInstance()->Add(_T("history"), param.GetWholeString());
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
	in->mParam = param;
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

int GroupCommand::EditDialog(HWND parent)
{
	// ダイアログを表示
	GroupEditDialog dlg(CWnd::FromHandle(parent));
	dlg.SetParam(in->mParam);
	dlg.mHotKeyAttr = in->mHotKeyAttr;

	if (dlg.DoModal() != IDOK) {
		return 1;
	}

	// 更新後の値を取得
	in->mParam = dlg.GetParam();
	in->mHotKeyAttr = dlg.mHotKeyAttr;

	// 変更後の内容で再登録
	auto cmdRepo = CommandRepository::GetInstance();
	cmdRepo->ReregisterCommand(this);

	return 0;
}

bool GroupCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mHotKeyAttr;
	return true;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool GroupCommand::IsPriorityRankEnabled()
{
	return true;
}

launcherapp::core::Command*
GroupCommand::Clone()
{
	auto clonedObj = std::make_unique<GroupCommand>();

	clonedObj->in->mParam = in->mParam;
	clonedObj->in->mErrMsg = in->mErrMsg;

	return clonedObj.release();
}

bool GroupCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);
	entry->Set(_T("Type"), GetType());

	const CommandParam& param = in->mParam;

	entry->Set(_T("Description"), GetDescription());
	entry->Set(_T("IsPassParam"), (bool)param.mIsPassParam);
	entry->Set(_T("IsRepeat"), (bool)param.mIsRepeat);
	entry->Set(_T("Repeats"), param.mRepeats);
	entry->Set(_T("IsConfirm"), (bool)param.mIsConfirm);

	entry->Set(_T("CommandCount"), (int)param.mItems.size());
	int index = 1;

	TCHAR key[128];
	for (auto& item : param.mItems) {

		_stprintf_s(key, _T("ItemName%d"), index);
		entry->Set(key, item.mItemName);
		_stprintf_s(key, _T("IsWait%d"), index);
		entry->Set(key, item.mIsWait);

		index++;
	}

	return true;
}

bool GroupCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	if (GetType() != entry->Get(_T("Type"), _T(""))) {
		return false;
	}

	CommandParam& param = in->mParam;

	param.mName = entry->GetName();
	param.mDescription = entry->Get(_T("Description"), _T(""));
	param.mIsPassParam = entry->Get(_T("IsPassParam"), false) ? TRUE : FALSE;
	param.mIsRepeat = entry->Get(_T("IsRepeat"), false) ? TRUE : FALSE;
	param.mRepeats = entry->Get(_T("Repeats"), 1);
	param.mIsConfirm = entry->Get(_T("IsConfirm"), true) ? TRUE : FALSE;

	int nItems = entry->Get(_T("CommandCount"), 0);
	if (nItems > 32) {  // 32を上限とする
		nItems = 32;
	}

	std::vector<GroupItem> items;

	TCHAR key[128];
	for (int i = 1; i <= nItems; ++i) {

		GroupItem item;

		_stprintf_s(key, _T("ItemName%d"), i);
		item.mItemName = entry->Get(key, _T(""));
		_stprintf_s(key, _T("IsWait%d"), i);
		item.mIsWait = entry->Get(key, false);

		items.push_back(item);
	}
	param.mItems.swap(items);

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mHotKeyAttr); 

	return true;
}

} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

