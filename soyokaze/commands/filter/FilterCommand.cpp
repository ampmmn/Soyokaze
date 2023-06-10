#include "pch.h"
#include "framework.h"
#include "FilterCommand.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "commands/filter/FilterCommandParam.h"
#include "commands/filter/CharConverter.h"
#include "commands/filter/FilterEditDialog.h"
#include "commands/filter/FilterDialog.h"
#include "core/CommandRepository.h"
#include "core/CommandHotKeyManager.h"
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

namespace soyokaze {
namespace commands {
namespace filter {


struct FilterCommand::PImpl
{
	PImpl() :
		mRefCount(1),
		mIsSilent(true)
	{
	}
	~PImpl()
	{
	}

	CommandParam mParam;
	CString mErrMsg;

	// エラーがあったときにメッセージを表示する
	bool mIsSilent;

	// 参照カウント
	uint32_t mRefCount;
};


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
	std::vector<CString> args;
	param.GetParameters(args);

	in->mErrMsg.Empty();

	// 子プロセスの出力を受け取るためのパイプを作成する
	Pipe pipeForStdout;
	Pipe pipeForStderr;

	PROCESS_INFORMATION pi = {};

	STARTUPINFO si = {};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdOutput = pipeForStdout.GetWriteHandle();
	si.hStdError = pipeForStderr.GetWriteHandle();
	si.wShowWindow = in->mParam.mShowType;

	CString path = in->mParam.mPath;
	ExpandArguments(path, args);
	ResolaveRelativeExePath(path);

	CString commandLine = _T(" ") + in->mParam.mParameter;
	ExpandArguments(commandLine, args);

	LPCTSTR workDir = NULL;
	if (in->mParam.mDir.GetLength() > 0) {
		workDir = in->mParam.mDir;
	}

	BOOL isOK = CreateProcess(path, commandLine.GetBuffer(commandLine.GetLength()), NULL, NULL, TRUE, 0, NULL, workDir, &si, &pi);
	commandLine.ReleaseBuffer();


	if (isOK == FALSE) {
		DWORD er = GetLastError();
		DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
		void* msgBuf;
		FormatMessage(flags, NULL, er, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msgBuf, 0, NULL);
		in->mErrMsg = (LPCTSTR)msgBuf;
		LocalFree(msgBuf);
		return FALSE;
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
	CString outputStr;
	conv.Convert(&output.front(), outputStr);

	CloseHandle(pi.hProcess);

	// 絞込みを行う
	FilterDialog dlg;
	dlg.SetCommandName(GetName());
	dlg.SetText(outputStr);

	if (dlg.DoModal() != IDOK) {
		return TRUE;
	}

	CString resultText = dlg.GetFilteredText();

	CString argSub = in->mParam.mAfterCommandParam;
	argSub.Replace(_T("$select"), resultText);

	CString parents;
	param.GetNamedParam(_T("PARENTS"), &parents);

	// 呼び出し元に自分自身を追加
	if (parents.IsEmpty() == FALSE) {
		parents += _T("/");
	}
	parents += GetName();

	Parameter paramSub;
	paramSub.AddArgument(argSub);
	paramSub.SetNamedParamString(_T("PARENTS"), parents);

	// Ctrlキーが押されているかを設定
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
		paramSub.SetNamedParamBool(_T("CtrlKeyPressed"), true);
	}

	if (in->mParam.mAfterType == 0) {
		// 他のコマンドを実行
		auto cmdRepo = CommandRepository::GetInstance();
		auto command = cmdRepo->QueryAsWholeMatch(in->mParam.mAfterCommandName, false);
		if (command) {
			command->Execute(paramSub);
			command->Release();
		}
	}
	else if (in->mParam.mAfterType == 1) {
		// 他のファイルを実行/URLを開く
		ShellExecCommand::ATTRIBUTE attr;
		attr.mPath = in->mParam.mAfterFilePath;
		attr.mParam = argSub;

		ShellExecCommand cmd;
		cmd.SetAttribute(attr);
		return cmd.Execute();

	}
	else if (in->mParam.mAfterType == 2) {
		// クリップボードにコピー

		SharedHwnd sharedWnd;
		if (OpenClipboard(sharedWnd.GetHwnd()) == FALSE) {
			return FALSE;
		}

		EmptyClipboard();

		size_t bufLen = sizeof(TCHAR) * (argSub.GetLength() + 1);
		HGLOBAL hMem = GlobalAlloc(GHND | GMEM_SHARE , bufLen);
		LPTSTR p = (LPTSTR)GlobalLock(hMem);
		_tcscpy_s(p, bufLen, argSub);
		GlobalUnlock(hMem);

		UINT type = sizeof(TCHAR) == 2 ? CF_UNICODETEXT : CF_TEXT;
		SetClipboardData(type , hMem);
		CloseClipboard();
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
	CString path = in->mParam.mPath;
	ExpandEnv(path);
	return IconLoader::Get()->LoadIconFromPath(path);
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
	cmdFile->Set(entry, _T("aftertype"), in->mParam.mAfterType);
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

