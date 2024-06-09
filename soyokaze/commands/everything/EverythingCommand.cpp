#include "pch.h"
#include "EverythingCommand.h"
#include "commands/everything/EverythingCommandEditDialog.h"
#include "commands/everything/EverythingResult.h"
#include "commands/everything/Everything-SDK/include/Everything.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "utility/ScopeAttachThreadInput.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "SharedHwnd.h"
#include <assert.h>
#include <regex>
#include <set>

namespace launcherapp {
namespace commands {
namespace everything {

using CommandRepositoryListenerIF = launcherapp::core::CommandRepositoryListenerIF;

struct EverythingCommand::PImpl
{
	CommandParam mParam;
	LONG mRefCount = 1;
};

CString EverythingCommand::GetType() { return _T("EverythingCommand"); }


EverythingCommand::EverythingCommand() : in(std::make_unique<PImpl>())
{
}

EverythingCommand::~EverythingCommand()
{
}

void EverythingCommand::Query(Pattern* pattern, std::vector<EverythingResult>& results)
{
	// コマンド名が一致しなければ候補を表示しない
	if (GetName().CompareNoCase(pattern->GetFirstWord()) != 0) {
		return;
	}

	// コマンドに設定されたオプションをEverythingの検索キーワードに置換する
	CString queryStr;
	switch(in->mParam.mTargetType) {
	case 1:
		queryStr += _T("file: ");
		break;
	case 2:
		queryStr += _T("folder: ");
		break;
	case 0:   // through
	default:
		// なし
		break;
	}

	queryStr += in->mParam.mBaseDir;
	queryStr += _T(" ");


	if (in->mParam.mIsMatchCase) {
		queryStr += _T("case:");
	}

	std::vector<CString> words;
	pattern->GetRawWords(words);
	for (size_t i = 1; i < words.size(); ++i) {
		queryStr += words[i];
		queryStr += _T(" ");
	}


	spdlog::debug(_T("EverythingQueryWord : {}"), (LPCTSTR)queryStr);
	Everything_SetSearch(queryStr);

	if (Everything_Query(TRUE) == FALSE) {
		return;
	}

	DWORD dwNumResults = Everything_GetNumResults();

	std::vector<EverythingResult> tmp;

	constexpr int LIMIT_TIME = 100;   // 結果取得にかける時間(これを超過したら打ち切り)
	DWORD start = GetTickCount();

	for (DWORD i = 0; i < dwNumResults; ++i) {

		TCHAR path[MAX_PATH_NTFS];
		if (Everything_GetResultFullPathName(i, path, MAX_PATH_NTFS) == EVERYTHING_ERROR_INVALIDCALL) {
			break;
		}

		EverythingResult result;
		result.mFullPath = path;
		result.mMatchLevel = Pattern::PartialMatch;

		tmp.push_back(result);

		if (i == dwNumResults || (GetTickCount() - start) >= LIMIT_TIME) {
			break;
		}
	}
	results.swap(tmp);
}


const CommandParam& EverythingCommand::GetParam()
{
	return in->mParam;
}

CString EverythingCommand::GetName()
{
	return in->mParam.mName;
}

CString EverythingCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString EverythingCommand::GetGuideString()
{
	return _T("キーワード入力すると候補を絞り込むことができます");
}


CString EverythingCommand::GetTypeDisplayName()
{
	return _T("Everything検索");
}

BOOL EverythingCommand::Execute(const Parameter& param)
{
	// コマンド名単体(後続のパラメータなし)で実行したときは、
	// コマンド名後ろに空白を入れた状態にして、検索ワードの入力を待つ状態にする

	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 2, 1, 0);

	auto cmdline = GetName();
	cmdline += _T(" ");
	SendMessage(sharedWnd.GetHwnd(), WM_APP+11, 0, (LPARAM)(LPCTSTR)cmdline);
	return TRUE;
}

CString EverythingCommand::GetErrorString()
{
	return _T("");
}

HICON EverythingCommand::GetIcon()
{
	// ToDo: Everythingのアイコン

	// 管理者権限でプロセスが動いている場合取れないので無効化
	// Everythingからアイコンがとれたらそれを使う
	// HWND h = FindWindow(_T("EVERYTHING_TASKBAR_NOTIFICATION"), NULL);
	// if (IsWindow(h)) {
	// 	return IconLoader::Get()->LoadIconFromHwnd(h);
	// }
	return IconLoader::Get()->GetImageResIcon(-5332);
}

int EverythingCommand::Match(Pattern* pattern)
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

int EverythingCommand::EditDialog(const Parameter*)
{
	// 設定変更画面を表示する
	SettingDialog dlg;
	dlg.SetParam(in->mParam);

	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	HOTKEY_ATTR hotKeyAttr;
	bool isGlobal = false;
	if (hotKeyManager->HasKeyBinding(GetName(), &hotKeyAttr, &isGlobal)) {
		dlg.SetHotKeyAttribute(hotKeyAttr, isGlobal);
	}

	if (dlg.DoModal() != IDOK) {
		return 0;
	}

	// 元の名前
	CString orgName = in->mParam.mName;

	// 変更後の設定値で上書き
	in->mParam = dlg.GetParam();

	// 名前の変更を登録しなおす
	auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
	cmdRepo->ReregisterCommand(this);

	// ホットキー設定を更新
	CommandHotKeyMappings hotKeyMap;
	hotKeyManager->GetMappings(hotKeyMap);

	hotKeyMap.RemoveItem(hotKeyAttr);

	dlg.GetHotKeyAttribute(hotKeyAttr, isGlobal);
	if (hotKeyAttr.IsValid()) {
		hotKeyMap.AddItem(GetName(), hotKeyAttr, isGlobal);
	}

	auto pref = AppPreference::Get();
	pref->SetCommandKeyMappings(hotKeyMap);
	pref->Save();

	return 0;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool EverythingCommand::IsPriorityRankEnabled()
{
	return true;
}

launcherapp::core::Command*
EverythingCommand::Clone()
{
	auto clonedCmd = std::make_unique<EverythingCommand>();

	clonedCmd->in->mParam = in->mParam;

	return clonedCmd.release();
}

bool EverythingCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	cmdFile->Set(entry, _T("description"), GetDescription());
	cmdFile->Set(entry, _T("BaseDir"), in->mParam.mBaseDir);

	return true;
}

bool EverythingCommand::NewDialog(
	const Parameter* param,
	EverythingCommand** newCmdPtr
)
{
	// パラメータ指定には対応していない
	// param;

	// 新規作成ダイアログを表示
	SettingDialog dlg;
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = dlg.GetParam();
	auto newCmd = std::make_unique<EverythingCommand>();
	newCmd->in->mParam = commandParam;

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

bool EverythingCommand::LoadFrom(CommandFile* cmdFile, void* e, EverythingCommand** newCmdPtr)
{
	ASSERT(newCmdPtr);

	CommandFile::Entry* entry = (CommandFile::Entry*)e;
	CString typeStr = cmdFile->Get(entry, _T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != EverythingCommand::GetType()) {
		return false;
	}

	CString name = cmdFile->GetName(entry);
	CString descriptionStr = cmdFile->Get(entry, _T("description"), _T(""));

	auto command = std::make_unique<EverythingCommand>();

	command->in->mParam.mName = name;
	command->in->mParam.mDescription = descriptionStr;

	command->in->mParam.mBaseDir = cmdFile->Get(entry, _T("BaseDir"), _T(""));

	if (newCmdPtr) {
		*newCmdPtr = command.release();
	}
	return true;
}

} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

