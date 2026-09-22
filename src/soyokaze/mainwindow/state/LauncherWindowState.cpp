#include "pch.h"
#include "mainwindow/state/LauncherWindowState.h"

using namespace launcherapp::mainwindow::state;

LauncherWindowState::LauncherWindowState(LauncherWindowStateContextIF* context) :
	mContext(context)
{
}

LauncherWindowState::~LauncherWindowState()
{
}

void LauncherWindowState::OnEnter()
{
}

void LauncherWindowState::OnExit()
{
}

void LauncherWindowState::OnActivate(bool isShowForce)
{
	UNREFERENCED_PARAMETER(isShowForce);
}

void LauncherWindowState::OnDeactivate()
{
}

void LauncherWindowState::OnExecuteRequested()
{
}

void LauncherWindowState::OnCancel()
{
}

void LauncherWindowState::OnContentCleared()
{
}

void LauncherWindowState::OnTextChanged()
{
}

void LauncherWindowState::OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result)
{
	UNREFERENCED_PARAMETER(result);
}

bool LauncherWindowState::OnKeyInput(unsigned int keyCode)
{
	UNREFERENCED_PARAMETER(keyCode);
	return false;
}

void LauncherWindowState::OnCandidateSelectionChanged(int index)
{
	UNREFERENCED_PARAMETER(index);
}

void LauncherWindowState::OnCandidateClicked()
{
}

void LauncherWindowState::OnCandidateDoubleClicked()
{
}

LauncherWindowStateContextIF* LauncherWindowState::GetContext() const
{
	return mContext;
}
