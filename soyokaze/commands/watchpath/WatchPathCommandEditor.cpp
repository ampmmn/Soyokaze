#include "pch.h"
#include "WatchPathCommandEditor.h"
#include "commands/watchpath/WatchPathCommandEditDialog.h"

namespace launcherapp {
namespace commands {
namespace watchpath {

struct CommandEditor::PImpl
{
	PImpl(CWnd* parentWnd) : mDialog(parentWnd)
	{
	}

	CommandEditDialog mDialog;
	CString mOrgName;
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

CString CommandEditor::GetOriginalName()
{
	return in->mOrgName;
}

// 名前を上書きする
void CommandEditor::OverrideName(LPCTSTR name) 
{
	in->mDialog.mParam.mName = name;
}

// 元のコマンド名を設定する(そのコマンド名と同じ場合は「コマンド名重複」とみなさない)
void CommandEditor::SetOriginalName(LPCTSTR name) 
{
	in->mOrgName = name;
	in->mDialog.SetOriginalName(name);
}

// コマンドを編集するためのダイアログを作成/取得する
bool CommandEditor::DoModal() 
{
	return in->mDialog.DoModal() == IDOK;
}


// UnknownIF
bool CommandEditor::QueryInterface(const launcherapp::core::IFID& ifid, void** obj) 
{
	if (ifid == IFID_WATCHPATHCOMMANDEDITOR) {
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

} // end of namespace watchpath
} // end of namespace commands
} // end of namespace launcherapp
