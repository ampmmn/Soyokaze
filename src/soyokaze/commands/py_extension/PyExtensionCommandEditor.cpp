#include "pch.h"
#include "PyExtensionCommandEditor.h"
#include "commands/py_extension/PyExtensionCommandEditDialog.h"
#include "commands/py_extension/ScintillaDLLLoader.h"
#include "python/PythonDLLLoader.h"

namespace launcherapp {
namespace commands {
namespace py_extension {

struct CommandEditor::PImpl
{
	PImpl(CWnd* parentWnd) : mDialog(parentWnd)
	{
	}

	CommandEditDialog mDialog;
	uint32_t mRefCount{1};
	
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

// 名前を上書きする
void CommandEditor::OverrideName(LPCTSTR name) 
{
	in->mDialog.mParam.mName = name;
	in->mDialog.mParam.mHotKeyAttr.Reset();
}

// 元のコマンド名を設定する(そのコマンド名と同じ場合は「コマンド名重複」とみなさない)
void CommandEditor::SetOriginalName(LPCTSTR name) 
{
	in->mDialog.SetOriginalName(name);
}

// コマンドを編集するためのダイアログを作成/取得する
bool CommandEditor::DoModal()
{
	if (PythonDLLLoader::Get()->IsAvailable() == false) {
		AfxMessageBox(_T("Python拡張コマンドを利用するには、Python(3.12以降)のパスを設定する必要があります。\n")
		              _T("アプリケーション設定からpython3.dllのパスを設定してください。"));
		return false;
	}
	if (ScintillaDLLLoader::GetInstance()->Initialize() == false) {
		AfxMessageBox(_T("Scintilla.dllがないためコマンド設定画面を表示できません。"));
		return false;
	}

	return in->mDialog.DoModal() == IDOK;
}


// UnknownIF
bool CommandEditor::QueryInterface(const launcherapp::core::IFID& ifid, void** obj) 
{
	if (ifid == IFID_PYEXTENSIONCOMMANDEDITOR) {
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

} // end of namespace alias
} // end of namespace commands
} // end of namespace launcherapp
