#include "pch.h"
#include "framework.h"
#include "UIAutomationAdhocCommand.h"
#include "core/IFIDDefine.h"
#include "commands/uiautomation/UIElementIndicatorWindow.h"
//#include "commands/uiautomation/UIAutomationElementWindow.h"
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

namespace launcherapp { namespace commands { namespace uiautomation {

using namespace launcherapp::commands::common;

struct UIAutomationAdhocCommand::PImpl
{
	PImpl() {}
	PImpl(RefPtr<UIElement>& elem) : mElem(elem) {} 

	RefPtr<UIElement> mElem;
	HWND mHwnd{nullptr};
	CString mPrefix;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(UIAutomationAdhocCommand)

UIAutomationAdhocCommand::UIAutomationAdhocCommand() : in(std::make_unique<PImpl>())
{
}

UIAutomationAdhocCommand::UIAutomationAdhocCommand(
	HWND hwnd,
	RefPtr<UIElement>& elem,
 	LPCTSTR prefix
) : in(std::make_unique<PImpl>(elem))
{
	this->mName = elem->GetName();
	in->mHwnd = hwnd;
	in->mPrefix = prefix;
	this->mDescription = this->mName;
}

UIAutomationAdhocCommand::~UIAutomationAdhocCommand()
{
}

CString UIAutomationAdhocCommand::GetName()
{
	CString name;
	if (in->mPrefix.IsEmpty() == FALSE) {
		name = in->mPrefix + _T(" ");
	}
	name += __super::GetName();

	return name;
}

CString UIAutomationAdhocCommand::GetTypeDisplayName()
{
	return _T("UI要素");
}

static void DoMouseOperation(CPoint point, bool isRight, bool isDblClk)
{
	DWORD downEvent = isRight ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_LEFTDOWN;
	DWORD upEvent = isRight ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_LEFTUP;

	SetCursorPos(point.x, point.y);

	// マウスクリックイベントを作成
	INPUT inputs[4] = {};
	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dwFlags = downEvent;
	inputs[0].mi.dx = point.x;
	inputs[0].mi.dy = point.y;

	inputs[1].type = INPUT_MOUSE;
	inputs[1].mi.dwFlags = upEvent;
	inputs[1].mi.dx = point.x;
	inputs[1].mi.dy = point.y;

	inputs[2] = inputs[0];
	inputs[3] = inputs[1];

	// イベントを送信
	SendInput(isDblClk ? 4 : 2, inputs, sizeof(INPUT));
}

bool UIAutomationAdhocCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (!in->mElem) {
		return false;
	}

	auto modifierFlags = hotkeyAttr.GetModifiers();
	if (modifierFlags == 0) {
		*action = new CallbackAction(_T("クリック"), [&](Parameter*, String*) -> bool {
			return in->mElem->Click();
		});
		return true;
	}
	else if (modifierFlags == MOD_SHIFT && in->mElem->CanFocus()) {
		*action = new CallbackAction(_T("フォーカス"), [&](Parameter*, String*) -> bool {
			return in->mElem->Focus();
		});
		return true;
	}


	return false;
}

HICON UIAutomationAdhocCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromHwnd(in->mHwnd);
}

launcherapp::core::Command*
UIAutomationAdhocCommand::Clone()
{
	return new UIAutomationAdhocCommand(in->mHwnd, in->mElem, in->mPrefix);
}

// メニューの項目数を取得する
int UIAutomationAdhocCommand::GetMenuItemCount()
{
	return 1;
}

// メニューの表示名を取得する
bool UIAutomationAdhocCommand::GetMenuItem(int index, Action** action)
{
	if (index != 0) {
		return false;
	}
	return GetAction(HOTKEY_ATTR(0, VK_RETURN), action);
}

// 選択された
void UIAutomationAdhocCommand::OnSelect(Command* prior)
{
	UNREFERENCED_PARAMETER(prior);

	if (IsWindow(in->mHwnd) == FALSE || IsWindowVisible(in->mHwnd) == FALSE) {
		return;
	}

	//対象のウインドウを強調表示する
	UIElementIndicatorWindow::GetInstance()->Indicate(in->mHwnd, in->mElem->GetRect());
}

// 選択解除された
void UIAutomationAdhocCommand::OnUnselect(Command* next)
{
	UNREFERENCED_PARAMETER(next);

	UIElementIndicatorWindow::GetInstance()->ClearIndication();

}

// 実行後のウインドウを閉じる方法を決定する
launcherapp::core::SelectionBehavior::CloseWindowPolicy
UIAutomationAdhocCommand::GetCloseWindowPolicy(uint32_t modifierMask)
{
	UNREFERENCED_PARAMETER(modifierMask);
	return launcherapp::core::SelectionBehavior::CLOSEWINDOW_ASYNC;
}

// 選択時に入力欄に設定するキーワードとキャレットを設定する
bool UIAutomationAdhocCommand::CompleteKeyword(CString&, int& , int& )
{
	return false;
}

bool UIAutomationAdhocCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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

}}} // end of namespace launcherapp::commands::uiautomation
