#include "pch.h"
#include "MainWindowSetTextAction.h"
#include "mainwindow/controller/MainWindowController.h"

namespace launcherapp { namespace actions { namespace mainwindow {

SetTextAction::SetTextAction(LPCTSTR displayName, LPCTSTR text) :
 	mDisplayName(displayName), mText(text)
{
}

SetTextAction::~SetTextAction()
{
}

// Action
// アクションの内容を示す名称
CString SetTextAction::GetDisplayName()
{
	return mDisplayName;
}

// アクションを実行する
bool SetTextAction::Perform(Parameter* param, String* errMsg)
{
	UNREFERENCED_PARAMETER(param);
	UNREFERENCED_PARAMETER(errMsg);

	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();

	bool isShowToggle = false;
	mainWnd->ActivateWindow(isShowToggle);
	mainWnd->SetText(mText);

	return true;
}

}}}

