#include "pch.h"
#include "framework.h"
#include "ClipboardHistoryCommand.h"
#include "commands/clipboardhistory/ClipboardPreviewWindow.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "mainwindow/controller/MainWindowController.h"
#include "icon/IconLoader.h"
#include "utility/StringUtil.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace clipboardhistory {


constexpr LAUNCHER_IFID IFID_CLIPBOARDHISTORYCOMMAND = 
{ 0x98d6ea5b, 0xfa57, 0x48d4, { 0xbf, 0x60, 0x4a, 0x1e, 0x39, 0xf1, 0x96, 0xda } };

struct ClipboardHistoryCommand::PImpl
{
	uint64_t mAppendDate{0};
	CString mPrefix;
	CString mData;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(ClipboardHistoryCommand)

ClipboardHistoryCommand::ClipboardHistoryCommand(
 	const CString& prefix,
	uint64_t appendDate,
 	const CString& data
) : 
	AdhocCommandBase(_T(""), _T("")),
	in(std::make_unique<PImpl>())
{
	in->mAppendDate = appendDate;
	in->mPrefix = prefix;
	in->mData = data;

	FILETIME sysFt;
	sysFt.dwHighDateTime = appendDate >> 32;
	sysFt.dwLowDateTime = appendDate & 0xFFFFFFFF;
	SYSTEMTIME st;
	FILETIME ft;
	FileTimeToLocalFileTime(&sysFt, &ft);
	FileTimeToSystemTime(&ft, &st);

	this->mDescription.Format(_T("%04d-%02d-%02d %02d:%02d"), 
			st.wYear,  st.wMonth, st.wDay, st.wHour, st.wMinute);
}

ClipboardHistoryCommand::~ClipboardHistoryCommand()
{
}

CString ClipboardHistoryCommand::GetName()
{
	return in->mPrefix + _T(" ") + in->mData;
}

CString ClipboardHistoryCommand::GetGuideString()
{
	return _T("⏎:コピー S-⏎:コピペ C-⏎:入力欄に貼り付け");
}

CString ClipboardHistoryCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

BOOL ClipboardHistoryCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	uint32_t state = GetModifierKeyState(param, MASK_CTRL | MASK_SHIFT);
	bool isCtrlPressed = (state & MASK_CTRL) != 0;
	bool isShiftPressed = (state & MASK_SHIFT) != 0;

	CString data = in->mData;

	// 値をコピー
	Clipboard::Copy(data);

	if (isShiftPressed) {
		// Shift-Insertキー押下による疑似的なペースト
    INPUT inputs[5] = {0};

    // Ctrlキー押下
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL; // 仮想キーコード: Ctrl
    inputs[0].ki.dwFlags = KEYEVENTF_KEYUP; // 離上イベント

    // Shiftキー押下
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_SHIFT; // 仮想キーコード: Shift

    // Insertキー押下
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = VK_INSERT; // 仮想キーコード: Insert

    // Insertキー離上
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_INSERT;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP; // 離上イベント

    // Shiftキー離上
    inputs[4].type = INPUT_KEYBOARD;
    inputs[4].ki.wVk = VK_SHIFT;
    inputs[4].ki.dwFlags = KEYEVENTF_KEYUP; // 離上イベント

    // イベント送信
    SendInput(5, inputs, sizeof(INPUT));
	}
	else if (isCtrlPressed) {
		// Ctrl-Enterで、入力欄にテキスト貼り付け
		auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
		bool isToggle = false;
		mainWnd->ActivateWindow(isToggle);

		mainWnd->SetText(data);

	}

	return TRUE;
}

HICON ClipboardHistoryCommand::GetIcon()
{
	return IconLoader::Get()->GetShell32Icon(-16763);
}

launcherapp::core::Command*
ClipboardHistoryCommand::Clone()
{
	return new ClipboardHistoryCommand(in->mPrefix, in->mAppendDate, in->mData);
}

// 選択された
void ClipboardHistoryCommand::OnSelect(Command* prior)
{
	UNREFERENCED_PARAMETER(prior);
	auto previewWindow = PreviewWindow::Get();
	previewWindow->Show();
	previewWindow->SetPreviewText(in->mData, in->mAppendDate);
}

// 選択解除された
void ClipboardHistoryCommand::OnUnselect(Command* next)
{
	bool shouldHidePreview = true;
	if (next) {
		ClipboardHistoryCommand* ptr = nullptr;
		if (next->QueryInterface(IFID_CLIPBOARDHISTORYCOMMAND, (void**)&ptr)) {
			ptr->Release();
			shouldHidePreview = false;
		}
	}

	if (shouldHidePreview == false) {
		return;
	}

	auto previewWindow = PreviewWindow::Get();
	previewWindow->Hide();
}

// 実行後のウインドウを閉じる方法を決定する
launcherapp::core::SelectionBehavior::CloseWindowPolicy
ClipboardHistoryCommand::GetCloseWindowPolicy()
{
	return launcherapp::core::SelectionBehavior::CLOSEWINDOW_SYNC;
}

// UnknownIF
bool ClipboardHistoryCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_SELECTIONBEHAVIOR) {
		AddRef();
		*cmd = (launcherapp::core::SelectionBehavior*)this;
		return true;
	}
	else if (ifid == IFID_CLIPBOARDHISTORYCOMMAND) {
		AddRef();
		*cmd = (ClipboardHistoryCommand*)this;
		return true;
	}

	return false;
}

CString ClipboardHistoryCommand::TypeDisplayName()
{
	return _T("クリップボード履歴");
}


} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp

