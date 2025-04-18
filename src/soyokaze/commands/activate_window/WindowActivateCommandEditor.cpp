#include "pch.h"
#include "WindowActivateCommandEditor.h"
#include "commands/activate_window/WindowActivateSettingDialog.h"

namespace launcherapp {
namespace commands {
namespace activate_window {

using namespace launcherapp::core;

struct WindowActivateCommandEditor::PImpl
{
	PImpl(CWnd* parentWnd) : mDialog(parentWnd)
	{
	}

	SettingDialog mDialog;
	uint32_t mRefCount{1};
	
};

WindowActivateCommandEditor::WindowActivateCommandEditor(CWnd* parentWnd) :
 	in(new PImpl(parentWnd))
{
}

WindowActivateCommandEditor::~WindowActivateCommandEditor()
{
}

void WindowActivateCommandEditor::SetParam(const CommandParam& param)
{
	return in->mDialog.SetParam(param);
}

const CommandParam& WindowActivateCommandEditor::GetParam()
{
	return in->mDialog.GetParam();
}

// 名前を上書きする
void WindowActivateCommandEditor::OverrideName(LPCTSTR name) 
{
	in->mDialog.SetName(name);
	in->mDialog.ResetHotKey();
}

// 元のコマンド名を設定する(そのコマンド名と同じ場合は「コマンド名重複」とみなさない)
void WindowActivateCommandEditor::SetOriginalName(LPCTSTR name) 
{
	in->mDialog.SetOriginalName(name);
}

// コマンドを編集するためのダイアログを作成/取得する
bool WindowActivateCommandEditor::DoModal() 
{
	return in->mDialog.DoModal() == IDOK;
}


// UnknownIF
bool WindowActivateCommandEditor::QueryInterface(const IFID& ifid, void** obj) 
{
	if (ifid == IFID_WINDOWACTIVATECOMMANDEDITOR) {
		*obj = (WindowActivateCommandEditor*)this;
		AddRef();
		return true;
	}
	return false;
}

uint32_t WindowActivateCommandEditor::AddRef() 
{
	return (uint32_t)InterlockedIncrement(&in->mRefCount);
}

uint32_t WindowActivateCommandEditor::Release() 
{
	auto n = InterlockedDecrement(&in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
}

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

