#include "pch.h"
#include "BuiltinCommandEditor.h"
#include "commands/builtin/BuiltinEditDialog.h"

namespace launcherapp {
namespace commands {
namespace builtin {

struct BuiltinCommandEditor::PImpl
{
	std::unique_ptr<BuiltinEditDialog> mDialog;
	uint32_t mRefCount = 1;
};

 BuiltinCommandEditor::BuiltinCommandEditor(
	const CString& name,
	const CString& description,
	bool canEditEnable,
	bool canEditConfirm,
	CWnd* parentWnd
) : in(new PImpl())
{
	in->mDialog.reset(new BuiltinEditDialog(name, description, canEditEnable, canEditConfirm, parentWnd)); 
}

BuiltinCommandEditor::~BuiltinCommandEditor()
{
}

CString BuiltinCommandEditor::GetName()
{
	return in->mDialog->GetName();
}

void BuiltinCommandEditor::SetEnable(bool isEnable)
{
	in->mDialog->SetEnable(isEnable);
}

bool BuiltinCommandEditor::GetEnable()
{
	return in->mDialog->GetEnable();
}

void BuiltinCommandEditor::SetConfirm(bool isConfirm)
{
	in->mDialog->SetConfirm(isConfirm);
}

bool BuiltinCommandEditor::GetConfirm()
{
	return in->mDialog->GetConfirm();
}

// 名前を上書きする
void BuiltinCommandEditor::OverrideName(LPCTSTR name) 
{
	in->mDialog->SetName(name);
}

// 元のコマンド名を設定する(そのコマンド名と同じ場合は「コマンド名重複」とみなさない)
void BuiltinCommandEditor::SetOriginalName(LPCTSTR name) 
{
	in->mDialog->SetOriginalName(name);
}

// コマンドを編集するためのダイアログを作成/取得する
bool BuiltinCommandEditor::DoModal() 
{
	return in->mDialog->DoModal() == IDOK;
}


// UnknownIF
bool BuiltinCommandEditor::QueryInterface(const launcherapp::core::IFID& ifid, void** obj) 
{
	if (ifid == IFID_BUILTINCOMMANDEDITOR) {
		*obj = (BuiltinCommandEditor*)this;
		AddRef();
		return true;
	}
	return false;
}

uint32_t BuiltinCommandEditor::AddRef() 
{
	return (uint32_t)InterlockedIncrement(&in->mRefCount);
}

uint32_t BuiltinCommandEditor::Release() 
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

