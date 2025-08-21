#include "pch.h"
#include "framework.h"
#include "PathExecuteCommand.h"
#include "commands/pathfind/ExcludePathList.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/common/SubProcess.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "utility/LocalPathResolver.h"
#include "utility/Path.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "mainwindow/controller/MainWindowController.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandNamedParameter = launcherapp::core::CommandNamedParameter;
using LocalPathResolver = launcherapp::utility::LocalPathResolver;
using ExecuteHistory = launcherapp::commands::common::ExecuteHistory;
using SubProcess = launcherapp::commands::common::SubProcess;

namespace launcherapp {
namespace commands {
namespace pathfind {

static CString EXE_EXT = _T(".exe");

using ShellExecCommand = launcherapp::commands::shellexecute::ShellExecCommand;

static const tregex& GetURLRegex()
{
	static tregex reg(_T("https?://.+"));
	return reg;
}

struct PathExecuteCommand::PImpl
{
	void Reload()
	{
		mResolver.ResetPath();

		// 追加パスを登録
		auto pref = AppPreference::Get();
		std::vector<CString> paths;
		pref->GetAdditionalPaths(paths);

		for (auto& path : paths) {
			mResolver.AddPath(path);
		}
	}

	LocalPathResolver mResolver;
	ExcludePathList* mExcludeFiles{nullptr};
	CString mWord;
	CString mFullPath;
	bool mIsURL{false};
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(PathExecuteCommand)

PathExecuteCommand::PathExecuteCommand(
	ExcludePathList* excludeList
) : in(std::make_unique<PImpl>())
{
	in->mExcludeFiles = excludeList;
}

PathExecuteCommand::~PathExecuteCommand()
{
}

void PathExecuteCommand::Reload()
{
	in->Reload();
}

CString PathExecuteCommand::GetName()
{
	if (in->mFullPath.IsEmpty()) {
		return _T("");
	}

	if (in->mIsURL) {
		return in->mFullPath;
	}

	return PathFindFileName(in->mFullPath);
}

CString PathExecuteCommand::GetGuideString()
{
	if (in->mIsURL) {
		return _T("⏎:ブラウザで開く");
	}
	else {
		return _T("⏎:開く C-⏎:フォルダを開く C-S-⏎:管理者権限で実行");
	}
}

CString PathExecuteCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

BOOL PathExecuteCommand::Execute(Parameter* param)
{
	if (in->mIsURL == false && Path::FileExists(in->mFullPath) == FALSE) {
		return FALSE;
	}

	// 履歴に追加
	auto namedParam = launcherapp::commands::common::GetCommandNamedParameter(param);
	if (namedParam->GetNamedParamBool(_T("RunAsHistory")) == false) {
		ExecuteHistory::GetInstance()->Add(_T("history"), param->GetWholeString());
	}

	SubProcess exec(param);
	SubProcess::ProcessPtr process;
	if (exec.Run(in->mFullPath, param->GetParameterString(), process) == FALSE) {
		//in->mErrMsg = (LPCTSTR)process->GetErrorMessage();
		return FALSE;
	}

	return TRUE;
}

HICON PathExecuteCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mFullPath);
}

int PathExecuteCommand::Match(Pattern* pattern)
{
	CString wholeWord = pattern->GetWholeString();
	wholeWord.Trim();

	int len = wholeWord.GetLength();

	// 先頭が "..." で囲われている場合は除去
	if (len >= 2 && 
	    wholeWord[0] == _T('"') && wholeWord[len-1] == _T('"')){
		wholeWord = wholeWord.Mid(1, len-2);
	}

	// URLパターンマッチするかを判定
	const tregex& regURL = GetURLRegex();
	if (std::regex_search((LPCTSTR)wholeWord, regURL)) {
		this->mDescription = wholeWord;

		in->mWord = wholeWord;
		in->mFullPath = wholeWord;
		in->mIsURL = true;
		return Pattern::WholeMatch;
	}

	// %が含まれている場合は環境変数が使われている可能性があるので展開を試みる
	if (wholeWord.Find(_T('%')) != -1) {
		DWORD sizeNeeded = ExpandEnvironmentStrings(wholeWord, nullptr, 0);
		std::vector<TCHAR> buf(sizeNeeded);
		ExpandEnvironmentStrings(wholeWord, buf.data(), sizeNeeded);
		wholeWord = buf.data();
	}

	CString filePart;

	int pos = 0;
	// "で始まる場合は対応する"まで切り出す
	if (wholeWord[pos] == _T('"')) {
		pos++;

		// 対応する"を探す
		while (pos < len) {
			if (wholeWord[pos] != _T('"')) {
				pos++;
				continue;
			}
			break;
		}
		if (pos == len) {
			// 対応する"がなかった
			return Pattern::Mismatch;
		}
		filePart = wholeWord.Mid(1, pos-1);
	}
	else {
		filePart = wholeWord;
	}

	in->mIsURL = false;

	// 絶対パス指定、かつ、存在するパスの場合は候補として表示
	if (PathIsRelative(filePart) == FALSE && Path::FileExists(filePart)) {
		this->mDescription = filePart;

		in->mWord = filePart;
		in->mFullPath = filePart;
		return Pattern::WholeMatch;
	}

	CString word = pattern->GetFirstWord();
	if (EXE_EXT.CompareNoCase(PathFindExtension(word)) != 0) {
		word += _T(".exe");
	}

	if (PathIsRelative(word) == FALSE) {
		return Pattern::Mismatch;
	}

	// 相対パスを解決する
	CString resolvedPath;
	if (in->mResolver.Resolve(word, resolvedPath)) {

		// 除外対象に含まれなければ
		if (in->mExcludeFiles &&
		    in->mExcludeFiles->Contains(resolvedPath) == false) {
			this->mDescription = resolvedPath;

			in->mWord = word;
			in->mFullPath = resolvedPath;
			return Pattern::WholeMatch;
		}
	}

	return Pattern::Mismatch;
}

launcherapp::core::Command*
PathExecuteCommand::Clone()
{
	auto clonedObj = make_refptr<PathExecuteCommand>();

	clonedObj->mDescription = this->mDescription;

	clonedObj->in->mResolver = in->mResolver;
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}

// メニューの項目数を取得する
int PathExecuteCommand::GetMenuItemCount()
{
	return in->mIsURL ? 2 : 5;
}

// メニューの表示名を取得する
bool PathExecuteCommand::GetMenuItemName(int index, LPCWSTR* displayNamePtr)
{
	if (in->mIsURL) {
		if (index == 0) {
			static LPCWSTR name = L"ブラウザで開く(&O)";
			*displayNamePtr= name;
			return true;
		}
		else if (index == 1) {
			static LPCWSTR name = L"URLをコマンドとして登録する(&U)";
			*displayNamePtr= name;
			return true;
		}
	}
	else {
		if (index == 0) {
			static LPCWSTR name = L"開く(&O)";
			*displayNamePtr= name;
			return true;
		}
		else if (index == 1) {
			static LPCWSTR name = L"フォルダを開く(&P)";
			*displayNamePtr= name;
			return true;
		}
		else if (index == 2) {
			static LPCWSTR name = L"管理者権限で実行(&A)";
			*displayNamePtr= name;
			return true;
		}
		else if (index == 3) {
			static LPCWSTR name = L"フルパスをコピー(&C)";
			*displayNamePtr= name;
			return true;
		}
		else if (index == 4) {
			static LPCWSTR name = L"プロパティ(&T)";
			*displayNamePtr= name;
			return true;
		}
	}
	return false;
}

// メニュー選択時の処理を実行する
bool PathExecuteCommand::SelectMenuItem(int index, launcherapp::core::CommandParameter* param)
{
	if (index < 0 || 4 < index) {
		return false;
	}

	if (index == 0) {
		return Execute(param) != FALSE;
	}

	RefPtr<CommandNamedParameter> namedParam;
	if (param->QueryInterface(IFID_COMMANDNAMEDPARAMETER, (void**)&namedParam) == false) {
		return false;
	}

	if (in->mIsURL) {
		if (index == 1) {
			// URLをコマンドとして登録

			// 登録用のコマンド文字列を生成
			CString cmdStr;
			cmdStr.Format(_T("new \"\" %s"), (LPCTSTR)in->mFullPath);

			auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
			bool isWaitSync = false;
			mainWnd->RunCommand((LPCTSTR)cmdStr, isWaitSync);
			return true;
		}
		else {
			return false;
		}
	}
	else {
		if (index == 1) {
			// パスを開くため、疑似的にCtrl押下で実行したことにする
			namedParam->SetNamedParamBool(_T("CtrlKeyPressed"), true);
			return Execute(param) != FALSE;
		}
		else if (index == 2)  {
			// 管理者権限で実行するため、疑似的にCtrl-Shift押下で実行したことにする
			namedParam->SetNamedParamBool(_T("ShiftKeyPressed"), true);
			namedParam->SetNamedParamBool(_T("CtrlKeyPressed"), true);
			return Execute(param) != FALSE;
		}
		else if (index == 3) {
			// クリップボードにコピー
			launcherapp::commands::common::Clipboard::Copy(in->mFullPath);
			return true;
		}
		else { // if (index == 4)
			// プロパティダイアログを表示
			SHObjectProperties(nullptr, SHOP_FILEPATH, in->mFullPath, nullptr);
			return true;
		}
	}
}

bool PathExecuteCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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

CString PathExecuteCommand::TypeDisplayName()
{
	static CString TEXT_TYPE_ADHOC((LPCTSTR)IDS_COMMAND_PATHEXEC);
	return TEXT_TYPE_ADHOC;
}


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace launcherapp

