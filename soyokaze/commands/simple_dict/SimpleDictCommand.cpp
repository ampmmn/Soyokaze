#include "pch.h"
#include "SimpleDictCommand.h"
#include "commands/simple_dict/SimpleDictEditDialog.h"
#include "commands/simple_dict/ExcelWrapper.h"
#include "commands/simple_dict/SimpleDictCommandUpdateListenerIF.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "utility/ScopeAttachThreadInput.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "commands/common/Message.h"
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "SharedHwnd.h"
#include <assert.h>
#include <regex>
#include <set>

namespace launcherapp {
namespace commands {
namespace simple_dict {

using CommandRepositoryListenerIF = launcherapp::core::CommandRepositoryListenerIF;

struct SimpleDictCommand::PImpl : public CommandRepositoryListenerIF
{
	void OnDeleteCommand(Command* command) override
	{
		if (command != mThisPtr) {
			return;
		}
		for (auto listener : mListeners) {
			listener->OnDeleteCommand(mThisPtr);
		}
	}
	void OnLancuherActivate() override {}
	void OnLancuherUnactivate() override {}

	SimpleDictCommand* mThisPtr;
	SimpleDictParam mParam;
	CommandHotKeyAttribute mHotKeyAttr;

	std::set<CommandUpdateListenerIF*> mListeners;
};

CString SimpleDictCommand::GetType() { return _T("SimpleDict"); }


SimpleDictCommand::SimpleDictCommand() : in(std::make_unique<PImpl>())
{
	in->mThisPtr = this;
	auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
	cmdRepo->RegisterListener(in.get());
}

SimpleDictCommand::~SimpleDictCommand()
{
	auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
	cmdRepo->UnregisterListener(in.get());
}

void SimpleDictCommand::AddListener(CommandUpdateListenerIF* listener)
{
	in->mListeners.insert(listener);
	listener->OnUpdateCommand(this, in->mParam.mName);
}

void SimpleDictCommand::RemoveListener(CommandUpdateListenerIF* listener)
{
	in->mListeners.erase(listener);
}

const SimpleDictParam& SimpleDictCommand::GetParam()
{
	return in->mParam;
}

CString SimpleDictCommand::GetName()
{
	return in->mParam.mName;
}

CString SimpleDictCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString SimpleDictCommand::GetGuideString()
{
	return _T("キーワード入力すると候補を絞り込むことができます");
}


CString SimpleDictCommand::GetTypeDisplayName()
{
	// コマンドとしてマッチしないが、キーワードマネージャに表示する文字列として使用する
	return _T("簡易辞書コマンド");
}

BOOL SimpleDictCommand::Execute(const Parameter& param)
{
	UNREFERENCED_PARAMETER(param);

	// コマンド名単体(後続のパラメータなし)で実行したときは簡易辞書の候補一覧を列挙させる

	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 2, 1, 0);

	auto cmdline = GetName();
	cmdline += _T(" ");
	SendMessage(sharedWnd.GetHwnd(), WM_APP+11, 0, (LPARAM)(LPCTSTR)cmdline);
	return TRUE;
}

CString SimpleDictCommand::GetErrorString()
{
	return _T("");
}

HICON SimpleDictCommand::GetIcon()
{
	// バインダーに矢印みたいなアイコン
	return IconLoader::Get()->GetImageResIcon(-5301);
}

int SimpleDictCommand::Match(Pattern* pattern)
{
	if (pattern->shouldWholeMatch() && pattern->Match(GetName()) == Pattern::WholeMatch) {
		// 内部のコマンド名マッチング用の判定
		return Pattern::WholeMatch;
	}
	else if (pattern->shouldWholeMatch() == false) {
		int level = pattern->Match(GetName());
		if (level == Pattern::FrontMatch) {
			return Pattern::FrontMatch;
		}
		if (level == Pattern::WholeMatch && pattern->GetWordCount() == 1) {
			// 入力欄からの入力で、前方一致するときは候補に出す
			return Pattern::WholeMatch;
		}
	}

	// 通常はこちら
	return Pattern::Mismatch;
}

int SimpleDictCommand::EditDialog(const Parameter*)
{
	// 設定変更画面を表示する
	SettingDialog dlg;
	dlg.SetParam(in->mParam);
	dlg.SetHotKeyAttribute(in->mHotKeyAttr);

	if (dlg.DoModal() != IDOK) {
		return 0;
	}

	// 元の名前
	CString orgName = in->mParam.mName;

	// 変更後の設定値で上書き
	in->mParam = dlg.GetParam();
	dlg.GetHotKeyAttribute(in->mHotKeyAttr);

	// 名前の変更を登録しなおす
	auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
	cmdRepo->ReregisterCommand(this);

	// コマンドの設定情報の変更を通知
	for (auto listener : in->mListeners) {
		listener->OnUpdateCommand(this, orgName);
	}
	return 0;
}

bool SimpleDictCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mHotKeyAttr;
	return true;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool SimpleDictCommand::IsPriorityRankEnabled()
{
	return true;
}

launcherapp::core::Command*
SimpleDictCommand::Clone()
{
	auto clonedCmd = std::make_unique<SimpleDictCommand>();

	clonedCmd->in->mParam = in->mParam;
	clonedCmd->in->mListeners = in->mListeners;

	return clonedCmd.release();
}

bool SimpleDictCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	entry->Set(_T("description"), GetDescription());
	entry->Set(_T("FilePath"), in->mParam.mFilePath);
	entry->Set(_T("SheetName"), in->mParam.mSheetName);
	entry->Set(_T("Range"), in->mParam.mRangeFront);
	entry->Set(_T("RangeBack"), in->mParam.mRangeBack);
	entry->Set(_T("IsFirstRowHeader"), (bool)in->mParam.mIsFirstRowHeader);
	entry->Set(_T("IsMatchWithoutKeyword"), (bool)in->mParam.mIsMatchWithoutKeyword);
	entry->Set(_T("IsEnableReverse"), (bool)in->mParam.mIsEnableReverse);
	entry->Set(_T("IsNotifyUpdate"), (bool)in->mParam.mIsNotifyUpdate);
	entry->Set(_T("IsExpandMacro"), (bool)in->mParam.mIsExpandMacro);
	entry->Set(_T("aftertype"), in->mParam.mActionType);
	entry->Set(_T("aftercommand"), in->mParam.mAfterCommandName);
	entry->Set(_T("afterfilepath"), in->mParam.mAfterFilePath);
	entry->Set(_T("afterparam"), in->mParam.mAfterCommandParam);

	return true;
}

bool SimpleDictCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != GetType()) {
		return false;
	}

	in->mParam.mName = entry->GetName();
	in->mParam.mDescription = entry->Get(_T("description"), _T(""));

	in->mParam.mFilePath = entry->Get(_T("FilePath"), _T(""));
	in->mParam.mSheetName = entry->Get(_T("SheetName"), _T(""));
	in->mParam.mRangeFront = entry->Get(_T("Range"), _T(""));
	in->mParam.mRangeBack = entry->Get(_T("RangeBack"), _T(""));
	in->mParam.mIsFirstRowHeader = entry->Get(_T("IsFirstRowHeader"), false);
	in->mParam.mIsMatchWithoutKeyword = entry->Get(_T("IsMatchWithoutKeyword"), true);
	in->mParam.mIsEnableReverse = entry->Get(_T("IsEnableReverse"), false);
	in->mParam.mIsNotifyUpdate = entry->Get(_T("IsNotifyUpdate"), false);
	in->mParam.mIsExpandMacro = entry->Get(_T("IsExpandMacro"), false);

	in->mParam.mActionType = entry->Get(_T("aftertype"), 2);
	in->mParam.mAfterCommandName = entry->Get(_T("aftercommand"), _T(""));
	in->mParam.mAfterFilePath = entry->Get(_T("afterfilepath"), _T(""));
	in->mParam.mAfterCommandParam = entry->Get(_T("afterparam"), _T("$value"));

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mHotKeyAttr); 

	return true;
}


bool SimpleDictCommand::NewDialog(
	const Parameter* param,
	SimpleDictCommand** newCmdPtr
)
{
	// パラメータ指定には対応していない
	UNREFERENCED_PARAMETER(param);


	ExcelApplication app;
	if (app.IsInstalled() == false) {
		AfxMessageBox(_T("簡易辞書コマンドはExcelがインストールされていないと利用できません"));
		return false;
	}

	// 新規作成ダイアログを表示
	SettingDialog dlg;
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = dlg.GetParam();
	auto newCmd = std::make_unique<SimpleDictCommand>();
	newCmd->in->mParam = commandParam;

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

