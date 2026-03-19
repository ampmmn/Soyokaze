#include "pch.h"
#include "framework.h"
#include "Win32MenuAdhocCommand.h"
#include "core/IFIDDefine.h"
#include "commands/uiautomation/UIElementIndicatorWindow.h"
#include "utility/ScopeAttachThreadInput.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/builtin/CallbackAction.h"
#include "icon/IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::actions::builtin;

namespace launcherapp { namespace commands { namespace win32menu {

using namespace launcherapp::commands::common;
using UIElementIndicatorWindow = launcherapp::commands::uiautomation::UIElementIndicatorWindow;

struct Win32MenuAdhocCommand::PImpl
{
	Win32MenuItemElement mElem;
	CString mPrefix;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(Win32MenuAdhocCommand)

Win32MenuAdhocCommand::Win32MenuAdhocCommand() : in(std::make_unique<PImpl>())
{
}

Win32MenuAdhocCommand::Win32MenuAdhocCommand(
	const Win32MenuItemElement& elem,
 	LPCTSTR prefix
) : in(std::make_unique<PImpl>())
{
	this->mDescription = this->mName;
	in->mPrefix = prefix;
	in->mElem = elem;
}

Win32MenuAdhocCommand::~Win32MenuAdhocCommand()
{
}

CString Win32MenuAdhocCommand::GetName()
{
	CString name;
	if (in->mPrefix.IsEmpty() == FALSE) {
		name = in->mPrefix + _T(" ");
	}
	name += in->mElem.GetName();

	return name;
}

CString Win32MenuAdhocCommand::GetTypeDisplayName()
{
	return _T("UI要素");
}

bool Win32MenuAdhocCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	auto modifierFlags = hotkeyAttr.GetModifiers();
	if (modifierFlags == 0) {
		*action = new CallbackAction(_T("メニュー選択"), [&](Parameter*, String*) -> bool {
			return in->mElem.Click();
		});
		return true;
	}

	return false;
}

HICON Win32MenuAdhocCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromHwnd(in->mElem.GetHwnd());
}

launcherapp::core::Command*
Win32MenuAdhocCommand::Clone()
{
	return new Win32MenuAdhocCommand(in->mElem, in->mPrefix);
}

// メニューの項目数を取得する
int Win32MenuAdhocCommand::GetMenuItemCount()
{
	return 1;
}

// メニューの表示名を取得する
bool Win32MenuAdhocCommand::GetMenuItem(int index, Action** action)
{
	if (index != 0) {
		return false;
	}
	return GetAction(HOTKEY_ATTR(0, VK_RETURN), action);
}

// 選択された
void Win32MenuAdhocCommand::OnSelect(Command* prior)
{
	UNREFERENCED_PARAMETER(prior);

	HWND hwnd = in->mElem.GetHwnd();

	if (IsWindow(hwnd) == FALSE || IsWindowVisible(hwnd) == FALSE) {
		return;
	}
	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	if (style & WS_MINIMIZE) {
		return;
	}

	//対象のウインドウを強調表示する
	RECT rc;
	GetWindowRect(hwnd, &rc);
	UIElementIndicatorWindow::GetInstance()->Indicate(hwnd, rc);
}

// 選択解除された
void Win32MenuAdhocCommand::OnUnselect(Command* next)
{
	UNREFERENCED_PARAMETER(next);

	UIElementIndicatorWindow::GetInstance()->ClearIndication();

}

// 実行後のウインドウを閉じる方法を決定する
launcherapp::core::SelectionBehavior::CloseWindowPolicy
Win32MenuAdhocCommand::GetCloseWindowPolicy(uint32_t modifierMask)
{
	UNREFERENCED_PARAMETER(modifierMask);
	return launcherapp::core::SelectionBehavior::CLOSEWINDOW_ASYNC;
}

// 選択時に入力欄に設定するキーワードとキャレットを設定する
bool Win32MenuAdhocCommand::CompleteKeyword(CString&, int& , int& )
{
	return false;
}

bool Win32MenuAdhocCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_CONTEXTMENUSOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ContextMenuSource*)this;
		return true;
	}
	if (ifid == IFID_SELECTIONBEHAVIOR) {
		AddRef();
		*cmd = (launcherapp::core::SelectionBehavior*)this;
		return true;
	}
	return false;
}

}}} // end of namespace launcherapp::commands::win32menu
