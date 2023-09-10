#include "pch.h"
#include "WindowActivateCommand.h"
#include "commands/activate_window/WindowActivateCommandParam.h"
#include "commands/activate_window/WindowActivateSettingDialog.h"
#include "core/CommandRepository.h"
#include "core/CommandHotKeyManager.h"
#include "utility/ScopeAttachThreadInput.h"
#include "AppPreference.h"
#include "CommandFile.h"
#include "IconLoader.h"
#include "resource.h"
#include <assert.h>
#include <regex>

namespace soyokaze {
namespace commands {
namespace activate_window {

constexpr int UPDATE_INTERVAL = 5000;

struct WindowActivateCommand::PImpl
{
	HWND FindHwnd();
	CommandParam mParam;

	CString mErrorMsg;

	HWND mCachedHwnd = nullptr;
	DWORD mLastUpdate = 0;
	uint32_t mRefCount = 1;
};

HWND WindowActivateCommand::PImpl::FindHwnd()
{
	if (IsWindow(mCachedHwnd) && GetTickCount() - mLastUpdate < UPDATE_INTERVAL) {
		return mCachedHwnd;
	}
	mCachedHwnd = nullptr;

	HWND hwnd = mParam.FindHwnd();
	if (hwnd) {
		mCachedHwnd = hwnd;
		mLastUpdate = GetTickCount();
	}
	return mCachedHwnd;
}

CString WindowActivateCommand::GetType() { return _T("WindowActivate"); }


WindowActivateCommand::WindowActivateCommand() : in(std::make_unique<PImpl>())
{
}

WindowActivateCommand::~WindowActivateCommand()
{
}

CString WindowActivateCommand::GetName()
{
	return in->mParam.mName;
}

CString WindowActivateCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString WindowActivateCommand::GetGuideString()
{
	return _T("Enter:ウインドウをアクティブにする");
}


CString WindowActivateCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMANDNAME_WINDOWACTIVATE);
	return TEXT_TYPE;
}

BOOL WindowActivateCommand::Execute(const Parameter& param)
{
	// ここで該当するウインドウを探す
	HWND hwndTarget = in->FindHwnd();

	if (IsWindow(hwndTarget) == FALSE) {
		in->mErrorMsg = _T("指定されたウインドウが見つかりません");
		return FALSE;
	}
	in->mErrorMsg.Empty();

	ScopeAttachThreadInput scope;

	LONG_PTR style = GetWindowLongPtr(hwndTarget, GWL_STYLE);
	if (param.GetNamedParamBool(_T("CtrlKeyPressed")) && (style & WS_MAXIMIZE) == 0) {
		// Ctrlキーが押されていたら最大化表示する
		PostMessage(hwndTarget, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}
	else if (style & WS_MINIMIZE) {
		// 最小化されていたら元に戻す
		PostMessage(hwndTarget, WM_SYSCOMMAND, SC_RESTORE, 0);
	}

	SetForegroundWindow(hwndTarget);
	return TRUE;
}

CString WindowActivateCommand::GetErrorString()
{
	return in->mErrorMsg;
}

HICON WindowActivateCommand::GetIcon()
{
	HWND hwndTarget = in->FindHwnd();

	if (IsWindow(hwndTarget) == FALSE) {
		return IconLoader::Get()->LoadUnknownIcon();
	}

	HICON icon = (HICON)GetClassLongPtr(hwndTarget, GCLP_HICON);
	if (icon) {
		return icon;
	}
	return (HICON)GetClassLongPtr(hwndTarget, GCLP_HICONSM);
}

int WindowActivateCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool WindowActivateCommand::IsEditable()
{
	return true;
}

int WindowActivateCommand::EditDialog(const Parameter*)
{
	SettingDialog dlg;

	auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
	auto param = in->mParam;

	HOTKEY_ATTR hotKeyAttr;
	bool isGlobal = false;
	if (hotKeyManager->HasKeyBinding(param.mName, &hotKeyAttr, &isGlobal)) {
		param.mHotKeyAttr = hotKeyAttr;
		param.mIsGlobal = isGlobal;
	}

	dlg.SetParam(param);
	if (dlg.DoModal() != IDOK) {
		return 0;
	}

	auto cmdNew = std::make_unique<WindowActivateCommand>();

	param = dlg.GetParam();
	cmdNew->in->mParam = param;

	// 名前が変わっている可能性があるため、いったん削除して再登録する
	auto cmdRepo = soyokaze::core::CommandRepository::GetInstance();
	cmdRepo->UnregisterCommand(this);
	cmdRepo->RegisterCommand(cmdNew.release());

	// ホットキー設定を更新
	CommandHotKeyMappings hotKeyMap;
	hotKeyManager->GetMappings(hotKeyMap);

	hotKeyMap.RemoveItem(hotKeyAttr);
	if (param.mHotKeyAttr.IsValid()) {
		hotKeyMap.AddItem(param.mName, param.mHotKeyAttr, param.mIsGlobal);
	}

	auto pref = AppPreference::Get();
	pref->SetCommandKeyMappings(hotKeyMap);

	pref->Save();
	return 0;
}

soyokaze::core::Command*
WindowActivateCommand::Clone()
{
	auto clonedCmd = std::make_unique<WindowActivateCommand>();

	clonedCmd->in->mParam = in->mParam;

	return clonedCmd.release();
}

bool WindowActivateCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());

	cmdFile->Set(entry, _T("description"), GetDescription());

	cmdFile->Set(entry, _T("CaptionStr"), in->mParam.mCaptionStr);
	cmdFile->Set(entry, _T("ClassStr"), in->mParam.mClassStr);
	cmdFile->Set(entry, _T("IsUseRegExp"), in->mParam.mIsUseRegExp != FALSE);

	return true;
}

uint32_t WindowActivateCommand::AddRef()
{
	return ++in->mRefCount;
}

uint32_t WindowActivateCommand::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

bool WindowActivateCommand::NewDialog(
	const Parameter* param,
	WindowActivateCommand** newCmdPtr
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
	auto newCmd = std::make_unique<WindowActivateCommand>();
	newCmd->in->mParam = commandParam;

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	// ホットキー設定を更新
	if (commandParam.mHotKeyAttr.IsValid()) {

		auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
		CommandHotKeyMappings hotKeyMap;
		hotKeyManager->GetMappings(hotKeyMap);

		hotKeyMap.AddItem(commandParam.mName, commandParam.mHotKeyAttr, commandParam.mIsGlobal);

		auto pref = AppPreference::Get();
		pref->SetCommandKeyMappings(hotKeyMap);

		pref->Save();
	}

	return true;
}

bool WindowActivateCommand::LoadFrom(CommandFile* cmdFile, void* e, WindowActivateCommand** newCmdPtr)
{
	ASSERT(newCmdPtr);

	CommandFile::Entry* entry = (CommandFile::Entry*)e;
	CString typeStr = cmdFile->Get(entry, _T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != WindowActivateCommand::GetType()) {
		return false;
	}

	CString name = cmdFile->GetName(entry);
	CString descriptionStr = cmdFile->Get(entry, _T("description"), _T(""));

	CString captionStr = cmdFile->Get(entry, _T("CaptionStr"), _T(""));
	CString classStr = cmdFile->Get(entry, _T("ClassStr"), _T(""));
	BOOL isUseRegExp = cmdFile->Get(entry, _T("IsUseRegExp"), false) ? TRUE : FALSE;


	auto command = std::make_unique<WindowActivateCommand>();

	command->in->mParam.mName = name;
	command->in->mParam.mDescription = descriptionStr;
	command->in->mParam.mCaptionStr = captionStr;
	command->in->mParam.mClassStr = classStr;
	command->in->mParam.mIsUseRegExp = isUseRegExp;

	if (command->in->mParam.BuildRegExp() == false) {
		return false;
	}
	if (newCmdPtr) {
		*newCmdPtr = command.release();
	}
	return true;
}

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace soyokaze

