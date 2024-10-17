#include "pch.h"
#include "BuiltinCommandEditor.h"
#include "commands/builtin/BuiltinEditDialog.h"

namespace launcherapp {
namespace commands {
namespace align_window {

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
	mDialog.reset(new BuiltinEditDialog(name, description, canEditEnable, canEditConfirm, parentWnd)); 
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

// コマンドは編集可能か?
void BuiltinCommandEditor::SetOriginalName(LPCTSTR name) 
{
	in->mDialog->SetOriginalName(name);
}

// コマンドを編集するためのダイアログを作成/取得する
bool BuiltinCommandEditor::DoModal() 
{
	return in->mDialog->DoModal() != IDOK;
}


// UnknownIF
bool BuiltinCommandEditor::QueryInterface(const IFID& ifid, void** obj) 
{
	if (ifid == IFID_ALIGNWINDOWCOMMANDEDITOR) {
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

