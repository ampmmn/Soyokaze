#include "pch.h"
#include "framework.h"
#include "ExitCommand.h"
#include "IconLoader.h"
#include "CommandFile.h"
#include "SharedHwnd.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

CString ExitCommand::GetType() { return _T("Builtin-Exit"); }

ExitCommand::ExitCommand(LPCTSTR name) : mRefCount(1)
{
	mName = name ? name : _T("exit");
}

ExitCommand::~ExitCommand()
{
}

CString ExitCommand::GetName()
{
	return mName;
}

CString ExitCommand::GetDescription()
{
	return _T("【終了】");
}

CString ExitCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_BUILTIN);
	return TEXT_TYPE;
}

BOOL ExitCommand::Execute(const Parameter& param)
{
	if (AfxGetMainWnd() != nullptr) {
		// AfxGetGetMainWnd()でウインドウがとれる==メインスレッドなので、
		// この場合はたんにPostQuitMessage(0)でOK
		PostQuitMessage(0);
		return TRUE;
	}
	else {
		// 別スレッド経由でコマンドが実行される場合があるので、
		// PostMessageでメインウインドウに配送することにより
		// メインウインドウ側スレッドの処理として終了処理を行う
		SharedHwnd sharedHwnd;
		HWND hwnd = sharedHwnd.GetHwnd();
		if (hwnd) {
			PostMessage(hwnd, WM_APP+8, 0, 0);
		}
		return TRUE;
	}
}

CString ExitCommand::GetErrorString()
{
	return _T("");
}

HICON ExitCommand::GetIcon()
{
	return IconLoader::Get()->LoadExitIcon();
}

int ExitCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool ExitCommand::IsEditable()
{
	return false;
}

int ExitCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command* ExitCommand::Clone()
{
	return new ExitCommand();
}

bool ExitCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);
	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	return true;
}

uint32_t ExitCommand::AddRef()
{
	return ++mRefCount;
}

uint32_t ExitCommand::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

}
}
}
