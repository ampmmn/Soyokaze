#include "pch.h"
#include "mainwindow/state/LauncherWindowStateBase.h"
#include "mainwindow/state/LauncherWindowStateContextIF.h"
#include "mainwindow/state/MainWindowHiddenState.h"

using namespace launcherapp::mainwindow::state;

LauncherWindowStateBase::LauncherWindowStateBase(LauncherWindowStateContextIF* context) :
	mContext(context)
{
}

LauncherWindowStateBase::~LauncherWindowStateBase()
{
}

void LauncherWindowStateBase::OnEnter()
{
}

void LauncherWindowStateBase::OnExit()
{
}

void LauncherWindowStateBase::OnActivate()
{
}

void LauncherWindowStateBase::OnDeactivate()
{
}

void LauncherWindowStateBase::OnShowRequested(bool isShowForce)
{
	auto context = GetContext();
	if (isShowForce || context->IsWindowVisibleFromState() == false) {
		context->ShowWindowFromState();
	}
	else if (context->IsWindowActive()) {
		if (context->IsShowToggleEnabled()) {
			context->ChangeState(std::make_unique<HiddenState>(context));
		}
	}
	else {
		context->ActivateVisibleWindow();
	}
}

void LauncherWindowStateBase::OnHideRequested()
{
}

void LauncherWindowStateBase::OnExecuteRequested()
{
}

void LauncherWindowStateBase::OnCancel()
{
}

void LauncherWindowStateBase::OnContentCleared()
{
}

void LauncherWindowStateBase::OnTextChanged()
{
}

void LauncherWindowStateBase::OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result)
{
	UNREFERENCED_PARAMETER(result);
}

bool LauncherWindowStateBase::OnKeyInput(unsigned int keyCode)
{
	UNREFERENCED_PARAMETER(keyCode);
	return false;
}

void LauncherWindowStateBase::OnCandidateSelectionChanged(int index)
{
	UNREFERENCED_PARAMETER(index);
}

void LauncherWindowStateBase::OnCandidateClicked()
{
}

void LauncherWindowStateBase::OnCandidateDoubleClicked()
{
}

void LauncherWindowStateBase::OnSelectionChanged()
{
}

void LauncherWindowStateBase::OnWindowGeometryChanged()
{
}

void LauncherWindowStateBase::OnExtraCandidateSelectionChanged()
{
}

void LauncherWindowStateBase::OnExtraCandidateClicked()
{
}

LauncherWindowStateContextIF* LauncherWindowStateBase::GetContext() const
{
	return mContext;
}
