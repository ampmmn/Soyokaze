#include "pch.h"
#include "UserCommandBase.h"
#include "commands/core/IFIDDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::core;

namespace launcherapp {
namespace commands {
namespace common {

UserCommandBase::UserCommandBase()
{
}

UserCommandBase::~UserCommandBase()
{
}

bool UserCommandBase::IsEditable()
{
	return true;
}

bool UserCommandBase::IsDeletable()
{
	return true;
}

// コマンドを編集するためのダイアログを作成/取得する
bool UserCommandBase::CreateEditor(HWND parent, CommandEditor** editor)
{
	UNREFERENCED_PARAMETER(parent);
	UNREFERENCED_PARAMETER(editor);

	ASSERT(0);    // ToDo: 派生クラス側で実装のこと
	return false;
}

// ダイアログ上での編集結果をコマンドに適用する
bool UserCommandBase::Apply(CommandEditor* editor)
{
	UNREFERENCED_PARAMETER(editor);

	ASSERT(0);    // ToDo: 派生クラス側で実装のこと
	return false;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool UserCommandBase::CreateNewInstanceFrom(CommandEditor* editor, Command** newCmd)
{
	UNREFERENCED_PARAMETER(editor);
	UNREFERENCED_PARAMETER(newCmd);
	ASSERT(0);    // ToDo: 派生クラス側で実装のこと
	return false;
}

bool UserCommandBase::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (ifid == IFID_EDITABLE) {
		AddRef();
		*cmd = (launcherapp::core::Editable*)this;
		return true;
	}
	if (ifid == IFID_COMMAND) {
		AddRef();
		*cmd = (launcherapp::core::Command*)this;
		return true;
	}
	return false;
}

bool UserCommandBase::CanExecute()
{
	return true;
}

CString UserCommandBase::GetErrorString()
{
	return _T("");
}

uint32_t UserCommandBase::AddRef()
{
	return ++mRefCount;
}

uint32_t UserCommandBase::Release()
{
	uint32_t n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

bool UserCommandBase::GetNamedParamString(Parameter* param, LPCTSTR name, CString& value)
{
	if (param== nullptr) {
		return false;
	}

	RefPtr<launcherapp::core::CommandNamedParameter> namedParam;
	if (param->QueryInterface(IFID_COMMANDNAMEDPARAMETER, (void**)&namedParam) == false) {
		return false;
	}

	int len = namedParam->GetNamedParamStringLength(name);
	if (len == 0) {
		return false;
	}

	namedParam->GetNamedParamString(name, value.GetBuffer(len), len);
	value.ReleaseBuffer();

	return true;
}

}
}
}

