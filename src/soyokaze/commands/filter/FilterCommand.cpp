#include "pch.h"
#include "framework.h"
#include "FilterCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/filter/FilterAdhocCommand.h"
#include "commands/filter/FilterCommandParam.h"
#include "commands/filter/FilterCommandEditor.h"
#include "commands/filter/FilterExecutor.h"
#include "commands/core/CommandRepository.h"
#include "matcher/PatternInternal.h"
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

using namespace launcherapp::commands::common;

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace filter {


struct FilterCommand::PImpl
{
	PImpl()
	{
	}
	~PImpl()
	{
		if (mExecutor) {
			mExecutor->Release();
		}
	}
	void LoadCandidates();

	CommandParam mParam;
	CString mErrMsg;
	//
	FilterExecutor* mExecutor{nullptr};

	bool mIsEmpty{false};
};

// 候補一覧の生成を開始する
void FilterCommand::PImpl::LoadCandidates()
{
	if (mExecutor == nullptr) {
		mExecutor = new FilterExecutor();
		mExecutor->LoadCandidates(mParam);
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString FilterCommand::GetType() { return _T("Filter"); }

FilterCommand::FilterCommand() : in(std::make_unique<PImpl>())
{
}

FilterCommand::~FilterCommand()
{
}

bool FilterCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (UserCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}
	if (ifid == IFID_EXTRACANDIDATESOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidateSource*)this;
		return true;
	}
	return false;
}

CString FilterCommand::GetName()
{
	return in->mParam.mName;
}


CString FilterCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString FilterCommand::GetGuideString()
{
	return _T("Enter:候補を表示する");
}

CString FilterCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_FILTERCOMMAND);
	return TEXT_TYPE;
}

BOOL FilterCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	if (in->mIsEmpty) {
		// 候補が存在しないとわかっている場合は何もしない
		return TRUE;
	}

	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 2, 1, 0);

	auto cmdline = GetName();
	cmdline += _T(" ");
	SendMessage(sharedWnd.GetHwnd(), WM_APP+11, 0, (LPARAM)(LPCTSTR)cmdline);
	return TRUE;
}

CString FilterCommand::GetErrorString()
{
	return in->mErrMsg;
}

FilterCommand& FilterCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;

	CString errMsg;
	in->mParam.BuildCandidateTextRegExp(errMsg);
	return *this;
}

FilterCommand& FilterCommand::GetParam(CommandParam& param)
{
	param = in->mParam;
	return *this;
}


HICON FilterCommand::GetIcon()
{
	return IconLoader::Get()->LoadPromptIcon();
}

int FilterCommand::Match(Pattern* pattern)
{
	in->mIsEmpty = false;

	if (pattern->shouldWholeMatch() && pattern->Match(GetName()) == Pattern::WholeMatch) {
		// 内部のコマンド名マッチング用の判定
		return Pattern::WholeMatch;
	}
	else if (pattern->shouldWholeMatch() == false) {

		// 入力欄からの入力で、前方一致するときは候補に出す
		int level = pattern->Match(GetName());
		if (level == Pattern::FrontMatch) {
			return Pattern::FrontMatch;
		}

		if (level == Pattern::WholeMatch) {

		 	if (pattern->GetWordCount() > 1) {
				return Pattern::HiddenMatch;
			}

			if (in->mExecutor && in->mExecutor->IsLoaded()) {

				// 候補列挙済の場合、候補が存在する場合は、候補をFilterAdhocCommandとして表示するため、
				// このコマンド自身は表示しない
				if (in->mExecutor->GetCandidatesCount() > 0) {
					// 候補一覧生成済の場合は表示しない
					return Pattern::Mismatch;
				}

				// 候補件数が0の場合、その旨をFilterCommand自身が表示する
				in->mIsEmpty = true;

				return Pattern::WholeMatch;
			}

			// このタイミングで候補一覧の生成を行う
			in->LoadCandidates();
			return Pattern::WholeMatch;
		}
	}

	// 通常はこちら
	return Pattern::Mismatch;
}

bool FilterCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
FilterCommand::Clone()
{
	auto clonedObj = make_refptr<FilterCommand>();
	clonedObj->SetParam(in->mParam);
	// mExecutorはコピーする必要があるか?
	return clonedObj.release();
}

bool FilterCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	return in->mParam.Save(entry);
}

bool FilterCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != FilterCommand::GetType()) {
		return false;
	}
	if (in->mParam.Load(entry) == false) {
		return false;
	}

	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(GetName(), &in->mParam.mHotKeyAttr);

	return true;
}

bool FilterCommand::NewDialog(Parameter* param, FilterCommand** newCmd)
{
	// 新規作成ダイアログを表示
	CString value;

	CommandParam tmpParam;

	GetNamedParamString(param, _T("COMMAND"), tmpParam.mName);
	GetNamedParamString(param, _T("PATH"), tmpParam.mPath);
	GetNamedParamString(param, _T("DESCRIPTION"), tmpParam.mDescription);
	GetNamedParamString(param, _T("ARGUMENT"), tmpParam.mParameter);

	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	cmdEditor->SetParam(tmpParam);
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto cmd = make_refptr<FilterCommand>();
	cmd->SetParam(cmdEditor->GetParam());

	if (newCmd) {
		*newCmd = cmd.get();
	}
	return true;
}

// コマンドを編集するためのダイアログを作成/取得する
bool FilterCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
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
bool FilterCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_FILTERCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	SetParam(cmdEditor->GetParam());

	// 設定変更を反映するため、候補のキャッシュを消す
	ClearCache();
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool FilterCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_FILTERCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	auto paramNew = cmdEditor->GetParam();

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<FilterCommand>();
	newCmd->SetParam(paramNew);

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

/**
 	コマンドの候補として追加表示する項目を取得する
 	@return true:取得成功   false:取得失敗(表示しない)
 	@param[in]  pattern  入力パターン
 	@param[out] commands 表示する候補
*/
bool FilterCommand::QueryCandidates(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	if (in->mExecutor == nullptr) {
		return false;
	}

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

	std::vector<FilterResult> results;
	in->mExecutor->Query(queryStr, results);

	for (auto& result : results) {
		int level = result.mMatchLevel;
		if (level == Pattern::PartialMatch) {
			// コマンド名が一致しているので少なくとも前方一致
			level = Pattern::FrontMatch;
		}
		commands.Add(CommandQueryItem(level, new FilterAdhocCommand(in->mParam, result)));
	}

	return true;
}

/**
 	追加候補を表示するために内部でキャッシュしているものがあれば、それを削除する
*/
void FilterCommand::ClearCache()
{
	if (in->mParam.mCacheType == 0) { // キャッシュしない場合
		SPDLOG_DEBUG(_T("clear cache."));
		if (in->mExecutor) {
			SPDLOG_DEBUG(_T("extcutor exists."));
			in->mExecutor->Release();
			in->mExecutor = nullptr;
		}
	}
	else {
		SPDLOG_DEBUG(_T("do not clear cache."));
	}
}


} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

