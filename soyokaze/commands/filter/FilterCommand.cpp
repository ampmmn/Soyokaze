#include "pch.h"
#include "framework.h"
#include "FilterCommand.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "commands/filter/FilterCommandParam.h"
#include "commands/common/CharConverter.h"
#include "commands/filter/FilterEditDialog.h"
#include "commands/filter/FilterDialog.h"
#include "core/CommandRepository.h"
#include "core/CommandHotKeyManager.h"
#include "utility/LocalPathResolver.h"
#include "utility/ScopeAttachThreadInput.h"
#include "CommandHotKeyMappings.h"
#include "AppPreference.h"
#include "CommandFile.h"
#include "IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include "utility/Pipe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace soyokaze::commands::common;
using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;

using CommandRepository = soyokaze::core::CommandRepository;
using LocalPathResolver = soyokaze::utility::LocalPathResolver;
using CharConverter = soyokaze::commands::common::CharConverter;

namespace soyokaze {
namespace commands {
namespace filter {


struct FilterCommand::PImpl
{
	PImpl() :
		mRefCount(1),
		mIsSilent(true),
		mExecutingCount(0)
	{
	}
	~PImpl()
	{
	}

	bool ExecutePreFilter(const std::vector<CString>& args, CString& src);
	bool ExecutePreFilterSubProcess(const std::vector<CString>& args, CString& src);
	bool ExecutePreFilterDefinedValue(const std::vector<CString>& args, CString& src);
	bool ExecutePreFilterClipboard(const std::vector<CString>& args, CString& src);

	bool ExecuteFilter(const CString& src, CString& dst);
	bool ExecutePostFilter(CString& dst, const Parameter& param);

	CommandParam mParam;
	CString mErrMsg;

	// エラーがあったときにメッセージを表示する
	bool mIsSilent;

	// 並列実行を許可しない場合の判定用のカウント
	int mExecutingCount;

	//
	bool mIsRunning;
	std::unique_ptr<FilterDialog> mDialog;

	// 参照カウント
	uint32_t mRefCount;
};

// 前段のプログラムを実行する
bool FilterCommand::PImpl::ExecutePreFilter(const std::vector<CString>& args, CString& src)
{
	if (mParam.mPreFilterType == 0) {
		// 子プロセスの出力から絞込み文字列を得る
		return ExecutePreFilterSubProcess(args, src);
	}
	else if (mParam.mPreFilterType == 1) {
		// クリップボードの値から
		return ExecutePreFilterClipboard(args, src);
	}
	else if (mParam.mPreFilterType == 2) {
		// 固定値
		return ExecutePreFilterDefinedValue(args, src);
	}
	
	return false;
}

bool FilterCommand::PImpl::ExecutePreFilterSubProcess(const std::vector<CString>& args, CString& src)
{
	// 子プロセスの出力を受け取るためのパイプを作成する
	Pipe pipeForStdout;
	Pipe pipeForStderr;

	PROCESS_INFORMATION pi = {};

	STARTUPINFO si = {};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdOutput = pipeForStdout.GetWriteHandle();
	si.hStdError = pipeForStderr.GetWriteHandle();
	si.wShowWindow = mParam.mShowType;

	CString path = mParam.mPath;

	ExpandEnv(path);
	ExpandArguments(path, args);

	LocalPathResolver resolver;
	resolver.Resolve(path);

	CString commandLine = _T(" ") + mParam.mParameter;
	ExpandArguments(commandLine, args);
	ExpandClipboard(commandLine);

	CString workDirStr;
	if (mParam.mDir.GetLength() > 0) {
		workDirStr = mParam.mDir;
		ExpandAfxCurrentDir(workDirStr);
	}

	LPCTSTR workDir = workDirStr.IsEmpty() ? nullptr : (LPCTSTR)workDirStr;
	BOOL isOK = CreateProcess(path, commandLine.GetBuffer(commandLine.GetLength()), NULL, NULL, TRUE, 0, NULL, workDir, &si, &pi);
	commandLine.ReleaseBuffer();


	if (isOK == FALSE) {
		DWORD er = GetLastError();
		DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
		void* msgBuf;
		FormatMessage(flags, NULL, er, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msgBuf, 0, NULL);
		mErrMsg = (LPCTSTR)msgBuf;
		LocalFree(msgBuf);
		return false;
	}

	CloseHandle(pi.hThread);

	HANDLE hRead = pipeForStdout.GetReadHandle();

	std::vector<char> output;
	std::vector<char> err;

	CharConverter conv;

	bool isExitChild = false;
	while(isExitChild == false) {

		if (isExitChild == false && WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) {
			isExitChild = true;

			DWORD exitCode;
			GetExitCodeProcess(pi.hProcess, &exitCode);

			if (exitCode != 0) {
				pipeForStderr.ReadAll(err);
				err.push_back(0x00);
				CString errStr;
				AfxMessageBox(conv.Convert(&err.front(), errStr));
				CloseHandle(pi.hProcess);
				return TRUE;
			}
		}

		pipeForStdout.ReadAll(output);
		pipeForStderr.ReadAll(err);
	}
	output.push_back(0x00);

	// ToDo: 文字コードを選べるようにする
	CString& outputStr = src;
	conv.Convert(&output.front(), outputStr);

	CloseHandle(pi.hProcess);

	return true;
}

bool FilterCommand::PImpl::ExecutePreFilterDefinedValue(const std::vector<CString>& args, CString& src)
{
	return true;
}

bool FilterCommand::PImpl::ExecutePreFilterClipboard(const std::vector<CString>& args, CString& src)
{
	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 10, 0, (LPARAM)&src);

	if (src.IsEmpty()) {
		mErrMsg = _T("クリップボードが空です");
		return false;
	}

	return true;
}

// 絞込み(フィルタ処理)の実行
bool FilterCommand::PImpl::ExecuteFilter(
	const CString& src,
	CString& dst
)
{
	FilterDialog& dlg = *mDialog.get();
	dlg.SetCommandName(mParam.mName);
	dlg.SetText(src);

	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// 後段の処理
	dst = dlg.GetFilteredText();

	return true;
}

bool FilterCommand::PImpl::ExecutePostFilter(
	CString& dst,
	const Parameter& param
)
{
	CString argSub = mParam.mAfterCommandParam;
	argSub.Replace(_T("$select"), dst);
	ExpandAfxCurrentDir(argSub);

	CString parents;
	param.GetNamedParam(_T("PARENTS"), &parents);

	// 呼び出し元に自分自身を追加
	if (parents.IsEmpty() == FALSE) {
		parents += _T("/");
	}
	parents += mParam.mName;

	Parameter paramSub;
	paramSub.AddArgument(argSub);
	paramSub.SetNamedParamString(_T("PARENTS"), parents);

	// Ctrlキーが押されているかを設定
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
		paramSub.SetNamedParamBool(_T("CtrlKeyPressed"), true);
	}

	if (mParam.mPostFilterType == 0) {
		// 他のコマンドを実行
		auto cmdRepo = CommandRepository::GetInstance();
		auto command = cmdRepo->QueryAsWholeMatch(mParam.mAfterCommandName, false);
		if (command) {
			command->Execute(paramSub);
			command->Release();
		}
	}
	else if (mParam.mPostFilterType == 1) {
		// 他のファイルを実行/URLを開く
		ShellExecCommand::ATTRIBUTE attr;

		attr.mPath = mParam.mAfterFilePath;
		attr.mPath.Replace(_T("$select"), dst);
		ExpandAfxCurrentDir(attr.mPath);

		attr.mParam = argSub;

		ShellExecCommand cmd;
		cmd.SetAttribute(attr);
		return cmd.Execute();

	}
	else if (mParam.mPostFilterType == 2) {
		// クリップボードにコピー
		size_t bufLen = sizeof(TCHAR) * (argSub.GetLength() + 1);
		HGLOBAL hMem = GlobalAlloc(GHND | GMEM_SHARE , bufLen);
		LPTSTR p = (LPTSTR)GlobalLock(hMem);
		_tcscpy_s(p, bufLen, argSub);
		GlobalUnlock(hMem);

		BOOL isSet=FALSE;
		SharedHwnd sharedWnd;
		SendMessage(sharedWnd.GetHwnd(), WM_APP + 9, (WPARAM)&isSet, (LPARAM)hMem);

		if (isSet == FALSE) {
			GlobalFree(hMem);
		}
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString FilterCommand::GetType() { return _T("Filter"); }

FilterCommand::FilterCommand() : in(new PImpl)
{
}

FilterCommand::~FilterCommand()
{
}

CString FilterCommand::GetName()
{
	return in->mParam.mName;
}


CString FilterCommand::GetDescription()
{
	return in->mParam.mDescription;
}

BOOL FilterCommand::Execute()
{
	Parameter param;
	return Execute(param);
}

BOOL FilterCommand::Execute(const Parameter& param)
{
	auto pref = AppPreference::Get();
	if (pref->IsArrowFilterCommandConcurrentRun() == false &&
	    in->mExecutingCount > 0) {
		// 同一コマンドの並列実行を許可しない

		// 先行して実行しているダイアログをアクティブにする
		HWND hwnd = in->mDialog ? in->mDialog->GetSafeHwnd() : nullptr;
		if (IsWindow(hwnd)) {
			ScopeAttachThreadInput scope;
			SetForegroundWindow(hwnd);
		}

		return TRUE;
	}

	struct scope_count {
		scope_count(int& c) : count(c) { ++count; }
		~scope_count() { --count; }
		int& count;
	} count_(in->mExecutingCount);

	in->mDialog.reset(new FilterDialog);

	std::vector<CString> args;
	param.GetParameters(args);

	if (args.size() > 0) {
		ExecuteHistory::GetInstance()->Add(_T("history"), param.GetWholeString());
	}

	in->mErrMsg.Empty();

	// 前段の処理を実行(フィルタリング対象の文字列を生成する)
	CString inputSrc;
	if (in->ExecutePreFilter(args, inputSrc) == false) {
		return FALSE;
	}

	// 絞込みを行う
	CString resultText;
	if (in->ExecuteFilter(inputSrc, resultText) == false) {
		return TRUE;
	}

	// 後段の処理を実行(絞込み結果に使って処理を行う)
	if (in->ExecutePostFilter(resultText, param) == false) {
		return FALSE;
	}

	return TRUE;
}

CString FilterCommand::GetErrorString()
{
	return in->mErrMsg;
}

FilterCommand& FilterCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;
	return *this;
}

FilterCommand& FilterCommand::GetParam(CommandParam& param)
{
	param = in->mParam;
	return *this;
}


HICON FilterCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(311);
}

int FilterCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool FilterCommand::IsEditable()
{
	return true;
}

int FilterCommand::EditDialog(const Parameter* param)
{
	FilterEditDialog dlg;
	dlg.SetOrgName(GetName());

	dlg.SetParam(in->mParam);
	auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
	HOTKEY_ATTR hotKeyAttr;
	bool isGlobal = false;
	if (hotKeyManager->HasKeyBinding(GetName(), &hotKeyAttr, &isGlobal)) {
		dlg.mHotKeyAttr = hotKeyAttr;
		dlg.mIsGlobal = isGlobal;
	}

	if (dlg.DoModal() != IDOK) {
		return 1;
	}

	FilterCommand* cmdNew = new FilterCommand();

	// 追加する処理
	CommandParam paramTmp;
	dlg.GetParam(paramTmp);
	cmdNew->SetParam(paramTmp);

	// 名前が変わっている可能性があるため、いったん削除して再登録する
	auto cmdRepo = soyokaze::core::CommandRepository::GetInstance();
	cmdRepo->UnregisterCommand(this);
	cmdRepo->RegisterCommand(cmdNew);

	// ホットキー設定を更新
	CommandHotKeyMappings hotKeyMap;
	hotKeyManager->GetMappings(hotKeyMap);

	hotKeyMap.RemoveItem(hotKeyAttr);
	if (dlg.mHotKeyAttr.IsValid()) {
		hotKeyMap.AddItem(paramTmp.mName, dlg.mHotKeyAttr, dlg.mIsGlobal);
	}

	auto pref = AppPreference::Get();
	pref->SetCommandKeyMappings(hotKeyMap);

	pref->Save();


	return 0;
}


soyokaze::core::Command*
FilterCommand::Clone()
{
	auto clonedObj = new FilterCommand();
	clonedObj->in->mParam = in->mParam;
	return clonedObj;
}

bool FilterCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());

	cmdFile->Set(entry, _T("description"), GetDescription());

	cmdFile->Set(entry, _T("path"), in->mParam.mPath);
	cmdFile->Set(entry, _T("dir"), in->mParam.mDir);
	cmdFile->Set(entry, _T("parameter"), in->mParam.mParameter);
	cmdFile->Set(entry, _T("show"), in->mParam.mShowType);
	cmdFile->Set(entry, _T("prefiltertype"), in->mParam.mPreFilterType);
	cmdFile->Set(entry, _T("aftertype"), in->mParam.mPostFilterType);
	cmdFile->Set(entry, _T("aftercommand"), in->mParam.mAfterCommandName);
	cmdFile->Set(entry, _T("afterfilepath"), in->mParam.mAfterFilePath);
	cmdFile->Set(entry, _T("afterparam"), in->mParam.mAfterCommandParam);

	return true;
}

uint32_t FilterCommand::AddRef()
{
	return ++in->mRefCount;
}

uint32_t FilterCommand::Release()
{
	auto n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace filter
} // end of namespace commands
} // end of namespace soyokaze

