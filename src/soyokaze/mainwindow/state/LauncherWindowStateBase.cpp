#include "pch.h"
#include "mainwindow/state/LauncherWindowStateBase.h"

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

void LauncherWindowStateBase::OnActivate(bool isShowForce)
{
	UNREFERENCED_PARAMETER(isShowForce);
}

void LauncherWindowStateBase::OnDeactivate()
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
