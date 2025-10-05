#include "pch.h"
#include "framework.h"
#include "ClipboardHistoryCommand.h"
#include "commands/clipboardhistory/ClipboardPreviewWindow.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "mainwindow/controller/MainWindowController.h"
#include "icon/IconLoader.h"
#include "utility/StringUtil.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using namespace launcherapp::actions::clipboard;

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

CString ClipboardHistoryCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool ClipboardHistoryCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0) {
		// コピーのみ
		*action = new CopyTextAction(in->mData);
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_SHIFT) {
		// コピー&ペースト
		bool enablePaste = true;
		*action = new CopyTextAction(in->mData, enablePaste);
		return true;
	}

	// 以下、無効化したコード
	// else if (isCtrlPressed) {
	// 	// Ctrl-Enterで、入力欄にテキスト貼り付け
	// 	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	// 	bool isToggle = false;
	// 	mainWnd->ActivateWindow(isToggle);

	// 	mainWnd->SetText(data);

	// }

	return false;
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
ClipboardHistoryCommand::GetCloseWindowPolicy(uint32_t )
{
	return launcherapp::core::SelectionBehavior::CLOSEWINDOW_ASYNC;
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

