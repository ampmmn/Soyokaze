#include "pch.h"
#include "framework.h"
#include "commands/builtin/BuiltinCommandBase.h"
#include "commands/builtin/BuiltinCommandEditor.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/IFIDDefine.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::core;

namespace launcherapp {
namespace commands {
namespace builtin {

BuiltinCommandBase::BuiltinCommandBase(LPCTSTR name) : 
	mName(name? name : _T("")), mRefCount(1)
{
}

BuiltinCommandBase::BuiltinCommandBase(const BuiltinCommandBase& rhs) :
	mName(rhs.mName),
	mDescription(rhs.mDescription),
	mError(rhs.mError),
	mRefCount(1),
	mIsConfirmBeforeRun(rhs.mIsConfirmBeforeRun),
	mCanSetConfirm(rhs.mCanSetConfirm),
	mIsEnable(rhs.mIsEnable),
	mCanDisable(rhs.mCanDisable)
{
}

BuiltinCommandBase::~BuiltinCommandBase()
{
}

bool BuiltinCommandBase::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (ifid == IFID_EDITABLE) {
		AddRef();
		*cmd = (launcherapp::core::Editable*)this;
		return true;
	}
	return false;
}

CString BuiltinCommandBase::GetName()
{
	return mName;
}

CString BuiltinCommandBase::GetDescription()
{
	return mDescription;
}

CString BuiltinCommandBase::GetGuideString()
{
	return _T("⏎:実行");
}

CString BuiltinCommandBase::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool BuiltinCommandBase::CanExecute()
{
	return true;
}

CString BuiltinCommandBase::GetErrorString()
{
	return mError;
}

HICON BuiltinCommandBase::GetIcon()
{
	return IconLoader::Get()->LoadDefaultIcon();
}

int BuiltinCommandBase::Match(Pattern* pattern)
{
	if (mIsEnable == false) {
		return Pattern::Mismatch;
	}
	return pattern->Match(GetName());
}

bool BuiltinCommandBase::IsAllowAutoExecute()
{
	return false;
}


bool BuiltinCommandBase::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	UNREFERENCED_PARAMETER(attr);

	// 組み込みコマンドは基本的にはホットキーを設定する機能を持たない
	return false;
}

bool BuiltinCommandBase::Save(CommandEntryIF* entry)
{
	ASSERT(entry);
	entry->Set(_T("Type"), GetType());

	entry->Set(_T("IsConfirmBeforeRun"), mIsConfirmBeforeRun);
	entry->Set(_T("IsEnable"), mIsEnable);

	return true;
}

bool BuiltinCommandBase::Load(CommandEntryIF* entry)
{
	ASSERT(entry);
	mIsConfirmBeforeRun = entry->Get(_T("IsConfirmBeforeRun"), false);
	mIsEnable = entry->Get(_T("IsEnable"), true);
	return true;
}

// コマンドは編集可能か?
bool BuiltinCommandBase::IsEditable()
{
	return (mCanDisable || mCanSetConfirm);
}

// コマンドは削除可能か?
bool BuiltinCommandBase::IsDeletable()
{
	// 組み込みコマンドは削除させない
	return false;
}

// コマンドを編集するためのダイアログを作成/取得する
bool BuiltinCommandBase::CreateEditor(HWND parent, CommandEditor** editor)
{
	if (mCanDisable == false && mCanSetConfirm == false) {
		return false;
	}

	auto cmdEditor = new BuiltinCommandEditor(GetName(), GetDescription(), mCanDisable, mCanSetConfirm, CWnd::FromHandle(parent));
	cmdEditor->SetEnable(mIsEnable);
	cmdEditor->SetConfirm(mIsConfirmBeforeRun);

	*editor = cmdEditor;

	return true;
}

// ダイアログ上での編集結果をコマンドに適用する
bool BuiltinCommandBase::Apply(CommandEditor* editor)
{
	RefPtr<BuiltinCommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_BUILTINCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	mName = cmdEditor->GetName();
	mIsEnable = cmdEditor->GetEnable();
	mIsConfirmBeforeRun = cmdEditor->GetConfirm();

	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool BuiltinCommandBase::CreateNewInstanceFrom(CommandEditor* editor, Command** newCmd)
{
	UNREFERENCED_PARAMETER(editor);
	UNREFERENCED_PARAMETER(newCmd);

	// 複製はサポートしない
	return false;
}

uint32_t BuiltinCommandBase::AddRef()
{
	return ++mRefCount;
}

uint32_t BuiltinCommandBase::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

void BuiltinCommandBase::LoadFrom(Entry* entry)
{
	Load(entry);
}

CString BuiltinCommandBase::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_BUILTIN);
	return TEXT_TYPE;
}

}
}
}

