#include "pch.h"
#include "framework.h"
#include "PlaceWindowInRegionAdhocCommand.h"
#include "core/IFIDDefine.h"
#include "commands/activate_window/ActivateIndicatorWindow.h"
#include "commands/place_window_in_region/RegionIndicatorWindow.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/builtin/CallbackAction.h"
#include "utility/ScopeAttachThreadInput.h"

#include "icon/IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ActivateIndicatorWindow = launcherapp::commands::activate_window::ActivateIndicatorWindow;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;

namespace launcherapp {
namespace commands {
namespace place_window_in_region {

using namespace launcherapp::commands::common;

struct PlaceWindowInRegionAdhocCommand::PImpl
{
	// 選択を解除
	void Unselect() {
		// 対象を強調表示
		ActivateIndicatorWindow::GetInstance()->Uncover();
		RegionIndicatorWindow::GetInstance()->Uncover();
	}


	HWND mHwnd{nullptr};
	CString mCaption;
	CommandParam mParam;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(PlaceWindowInRegionAdhocCommand)

PlaceWindowInRegionAdhocCommand::PlaceWindowInRegionAdhocCommand(
	HWND hwnd,
	LPCTSTR name,
 	const CommandParam& param
) : in(std::make_unique<PImpl>())
{
	in->mHwnd = hwnd;
	in->mCaption = name;
	in->mParam = param;

	this->mName.Format(_T("%s %s"), (LPCTSTR)param.mName, name);
	this->mDescription = param.mDescription;
}

PlaceWindowInRegionAdhocCommand::~PlaceWindowInRegionAdhocCommand()
{
}

CString PlaceWindowInRegionAdhocCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

// 修飾キー押下状態に対応した実行アクションを取得する
bool PlaceWindowInRegionAdhocCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() != 0) {
		return false;
	}

	*action = new CallbackAction(_T("ウインドウを配置する"), [&](Parameter*,String*) -> bool {

		ScopeAttachThreadInput scope;
		auto& wp = in->mParam.mPlacement;
		SetWindowPlacement(in->mHwnd, &wp);
		// Zオーダーを前面に移動
		if (in->mParam.mIsActivate) {
			SetForegroundWindow(in->mHwnd);
		}
		return true;
	});
	return true;
}

HICON PlaceWindowInRegionAdhocCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromHwnd(in->mHwnd);
}

launcherapp::core::Command*
PlaceWindowInRegionAdhocCommand::Clone()
{
	auto cmd = new PlaceWindowInRegionAdhocCommand(in->mHwnd, in->mCaption, in->mParam);
	return cmd;
}

// 選択された
void PlaceWindowInRegionAdhocCommand::OnSelect(Command* prior)
{
	UNREFERENCED_PARAMETER(prior);

	if (IsWindow(in->mHwnd) == FALSE || IsWindowVisible(in->mHwnd) == FALSE) {
		return;
	}

	// ToDo: 領域を強調表示

	// ランチャーウインドウの後ろに表示するため、現在ランチャーウインドウの直後にあるウインドウを取得
	SharedHwnd mainWnd;
	auto hwndPrev = GetWindow(mainWnd.GetHwnd(), GW_HWNDNEXT);

	// SetWindowPosでZOrderを変更すると、WS_EX_TOPMOST属性がつくことがあるので復元できるよう属性を保持する
	bool hasTopMost1 = (GetWindowLongPtr(in->mHwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;

	// ランチャーの直後にあるウインドウの前に対象ウインドウをZOrderのみ移動する
	SetWindowPos(in->mHwnd, hwndPrev, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

	bool hasTopMost2 = (GetWindowLongPtr(in->mHwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;
	if (hasTopMost1 != hasTopMost2) {
		// WS_EX_TOPMOST属性をもとに戻す
		SetWindowPos(in->mHwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	}

	// 対象を強調表示
	ActivateIndicatorWindow::GetInstance()->Cover(in->mHwnd);
	RegionIndicatorWindow::GetInstance()->Cover(in->mParam.mPlacement.rcNormalPosition);
}

// 選択解除された
void PlaceWindowInRegionAdhocCommand::OnUnselect(Command* next)
{
	UNREFERENCED_PARAMETER(next);

	// OnSelectで強調表示したものをもとに戻す
	in->Unselect();
}

// 実行後のウインドウを閉じる方法を決定する
launcherapp::core::SelectionBehavior::CloseWindowPolicy
PlaceWindowInRegionAdhocCommand::GetCloseWindowPolicy(uint32_t modifierFlags)
{
	if (modifierFlags == (MOD_CONTROL | MOD_SHIFT)) {
		return launcherapp::core::SelectionBehavior::CLOSEWINDOW_NOCLOSE;
	}
	else {
		return launcherapp::core::SelectionBehavior::CLOSEWINDOW_SYNC;
	}
}

// 選択時に入力欄に設定するキーワードとキャレットを設定する
bool PlaceWindowInRegionAdhocCommand::CompleteKeyword(CString& , int& , int& )
{
	return false;
}

bool PlaceWindowInRegionAdhocCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_SELECTIONBEHAVIOR) {
		AddRef();
		*cmd = (launcherapp::core::SelectionBehavior*)this;
		return true;
	}
	return false;
}

CString PlaceWindowInRegionAdhocCommand::TypeDisplayName()
{
	return _T("ウインドウ配置コマンド");
}


} // end of namespace place_window_in_region
} // end of namespace commands
} // end of namespace launcherapp

