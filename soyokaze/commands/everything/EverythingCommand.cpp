#include "pch.h"
#include "EverythingCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/everything/EverythingCommandEditor.h"
#include "commands/everything/EverythingAdhocCommand.h"
#include "commands/everything/EverythingResult.h"
#include "commands/everything/EverythingProxy.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "commands/core/CommandFile.h"
#include "matcher/PatternInternal.h"
#include "utility/ScopeAttachThreadInput.h"
#include "setting/AppPreference.h"

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

	bool mShouldComletion = false;
	LONG mRefCount = 1;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CString EverythingCommand::GetType() { return _T("EverythingCommand"); }


EverythingCommand::EverythingCommand() : in(std::make_unique<PImpl>())
{
}

EverythingCommand::~EverythingCommand()
{
}

bool EverythingCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (__super::QueryInterface(ifid, cmd)) {
		return true;
	}
	if (ifid == IFID_EXTRACANDIDATESOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidateSource*)this;
		return true;
	}
	return false;
}

void EverythingCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;
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
	return _T("キーワード入力するとEverything検索結果を表示します");
}

CString EverythingCommand::GetTypeDisplayName()
{
	return _T("Everything検索");
}

BOOL EverythingCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	if (in->mShouldComletion) {
		// コマンド名単体(後続のパラメータなし)で実行したときは、
		// コマンド名後ろに空白を入れた状態にして、検索ワードの入力を待つ状態にする

		SharedHwnd sharedWnd;
		SendMessage(sharedWnd.GetHwnd(), WM_APP + 2, 1, 0);

		auto cmdline = GetName();
		cmdline += _T(" ");
		SendMessage(sharedWnd.GetHwnd(), WM_APP+11, 0, (LPARAM)(LPCTSTR)cmdline);
		return TRUE;
	}

	auto proxy = EverythingProxy::Get();
	if (proxy->GetLastMethod() == 1) {
		proxy->ActivateMainWindow();
	}
	return TRUE;
}

CString EverythingCommand::GetErrorString()
{
	return _T("");
}

HICON EverythingCommand::GetIcon()
{
	return EverythingProxy::Get()->GetIcon();
}

int EverythingCommand::Match(Pattern* pattern)
{
	in->mShouldComletion = false;

	if (pattern->shouldWholeMatch() && pattern->Match(GetName()) == Pattern::WholeMatch) {
		// 内部のコマンド名マッチング用の判定
		in->mShouldComletion = true;
		return Pattern::WholeMatch;
	}
	else if (pattern->shouldWholeMatch() == false) {

		auto proxy = EverythingProxy::Get();

		// APIもWMも利用しない場合はマッチさせない
		if (proxy->IsUseAPI() == false && proxy->IsUseWM() == false) {
			return Pattern::Mismatch;
		}

		int level = pattern->Match(GetName());
		if (level == Pattern::FrontMatch) {
			in->mShouldComletion = true;
			return Pattern::FrontMatch;
		}

		// API利用の場合は簡易辞書コマンド的な動作にする
		if (proxy->IsUseWM() == false) {
			if (level == Pattern::WholeMatch && pattern->GetWordCount() == 1) {
				// 入力欄からの入力で、前方一致するときは候補に出す
				in->mShouldComletion = true;
				return Pattern::WholeMatch;
			}
		}
		else {
			return level;
		}
	}
	// 通常はこちら
	return Pattern::Mismatch;
}

bool EverythingCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
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

bool EverythingCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	entry->Set(_T("description"), GetDescription());
	entry->Set(_T("BaseDir"), in->mParam.mBaseDir);
	entry->Set(_T("TargetType"), in->mParam.mTargetType);
	entry->Set(_T("IsMatchCase"), in->mParam.mIsMatchCase);
	entry->Set(_T("IsRegex"), in->mParam.mIsRegex);
	entry->Set(_T("OtherParam"), in->mParam.mOtherParam);

	return true;
}

bool EverythingCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != EverythingCommand::GetType()) {
		return false;
	}

	in->mParam.mName = entry->GetName();
	in->mParam.mDescription = entry->Get(_T("description"), _T(""));
	in->mParam.mBaseDir = entry->Get(_T("BaseDir"), _T(""));
	in->mParam.mTargetType = entry->Get(_T("TargetType"), 0);
	in->mParam.mIsMatchCase = entry->Get(_T("IsMatchCase"), false);
	in->mParam.mIsRegex = entry->Get(_T("IsRegex"), false);
	in->mParam.mOtherParam = entry->Get(_T("OtherParam"), _T(""));

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mParam.mHotKeyAttr); 

	return true;
}

bool EverythingCommand::NewDialog(
	Parameter* param,
	EverythingCommand** newCmdPtr
)
{
	UNREFERENCED_PARAMETER(param);

	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = cmdEditor->GetParam();
	auto newCmd = std::make_unique<EverythingCommand>();
	newCmd->in->mParam = commandParam;

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

bool EverythingCommand::LoadFrom(CommandFile* cmdFile, void* e, EverythingCommand** newCmdPtr)
{
	UNREFERENCED_PARAMETER(cmdFile);

	ASSERT(newCmdPtr);

	CommandFile::Entry* entry = (CommandFile::Entry*)e;

	auto command = std::make_unique<EverythingCommand>();
	if (command->Load(entry) == false) {
		return false;
	}

	if (newCmdPtr) {
		*newCmdPtr = command.release();
	}
	return true;
}

// コマンドを編集するためのダイアログを作成/取得する
bool EverythingCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
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
bool EverythingCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_EVERYTHINGCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	in->mParam = cmdEditor->GetParam();
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool EverythingCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_EVERYTHINGCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	auto paramNew = cmdEditor->GetParam();

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = std::make_unique<EverythingCommand>();
	newCmd->SetParam(paramNew);

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}


bool EverythingCommand::QueryCandidates(Pattern* pattern, CommandQueryItemList& commands)
{
	// コマンド名が一致しなければ候補を表示しない
	if (GetName().CompareNoCase(pattern->GetFirstWord()) != 0) {
		return false;
	}

	RefPtr<PatternInternal> pat2;
	if (pattern->QueryInterface(IFID_PATTERNINTERNAL, (void**)&pat2) == false) {
		return false;
	}

 	std::vector<CString> words;
	CString queryStr;
	pat2->GetRawWords(words);
	for (size_t i = 1; i < words.size(); ++i) {
		queryStr += words[i];
		queryStr += _T(" ");
	}

	queryStr = in->mParam.BuildQueryString(queryStr);

	std::vector<EverythingResult> results;
	EverythingProxy::Get()->Query(queryStr, results);

	for (auto& result : results) {
		commands.Add(CommandQueryItem(result.mMatchLevel, new EverythingAdhocCommand(in->mParam, result)));
	}
	return true;
}

void EverythingCommand::ClearCache()
{
}


} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

