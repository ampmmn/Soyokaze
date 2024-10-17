#include "pch.h"
#include "AlignWindowCommandEditor.h"
#include "commands/align_window/AlignWindowSettingDialog.h"

namespace launcherapp {
namespace commands {
namespace align_window {

using namespace launcherapp::core;

struct CommandEditor::PImpl
{
	PImpl(CWnd* parentWnd) : mDialog(parentWnd)
	{
	}

	SettingDialog mDialog;
	uint32_t mRefCount = 1;
	
};

CommandEditor::CommandEditor(CWnd* parentWnd) :
 	in(new PImpl(parentWnd))
{
}

CommandEditor::~CommandEditor()
{
}

void CommandEditor::SetParam(const CommandParam& param)
{
	return in->mDialog.SetParam(param);
}

const CommandParam& CommandEditor::GetParam()
{
	return in->mDialog.GetParam();
}

// コマンドは編集可能か?
void CommandEditor::SetOriginalName(LPCTSTR name) 
{
	in->mDialog.SetOriginalName(name);
}

// コマンドを編集するためのダイアログを作成/取得する
bool CommandEditor::DoModal() 
{
	return in->mDialog.DoModal() != IDOK;
}


// UnknownIF
bool CommandEditor::QueryInterface(const IFID& ifid, void** obj) 
{
	if (ifid == IFID_ALIGNWINDOWCOMMANDEDITOR) {
		*obj = (CommandEditor*)this;
		AddRef();
		return true;
	}
	return false;
}

uint32_t CommandEditor::AddRef() 
{
	return (uint32_t)InterlockedIncrement(&in->mRefCount);
}

uint32_t CommandEditor::Release() 
{
	auto n = InterlockedDecrement(&in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
}

} // end of namespace align_window
} // end of namespace commands
} // end of namespace launcherapp

