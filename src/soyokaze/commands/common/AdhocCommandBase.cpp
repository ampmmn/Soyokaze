#include "pch.h"
#include "framework.h"
#include "AdhocCommandBase.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace common {

AdhocCommandBase::AdhocCommandBase(LPCTSTR name, LPCTSTR description) : 
	mName(name),
	mDescription(description)
{
}

AdhocCommandBase::~AdhocCommandBase()
{
}

bool AdhocCommandBase::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (ifid == IFID_COMMAND) {
		AddRef();
		*cmd = (launcherapp::core::Command*)this;
		return true;
	}
	return false;
}


CString AdhocCommandBase::GetName()
{
	return mName;
}

CString AdhocCommandBase::GetDescription()
{
	return mDescription;
}

CString AdhocCommandBase::GetGuideString()
{
	return _T("");
}

bool AdhocCommandBase::CanExecute()
{
	return true;
}

BOOL AdhocCommandBase::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// 派生側で実装する
	ASSERT(0);
	return TRUE;
}

// 修飾キー押下状態に対応した実行アクションを取得する
bool AdhocCommandBase::GetAction(uint32_t modifierFlags, Action** action)
{
	UNREFERENCED_PARAMETER(modifierFlags);
	UNREFERENCED_PARAMETER(action);

	// 派生側で実装する
	return false;
}

CString AdhocCommandBase::GetErrorString()
{
	return mErrMsg;
}

HICON AdhocCommandBase::GetIcon()
{
	return IconLoader::Get()->LoadUnknownIcon();
}

int AdhocCommandBase::Match(Pattern* pattern)
{
	// 必要に応じて派生側で実装する
	return pattern->Match(this->mName);
}

bool AdhocCommandBase::IsAllowAutoExecute()
{
	return false;
}

bool AdhocCommandBase::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	UNREFERENCED_PARAMETER(attr);

	// 一時コマンドはホットキー設定を持たない
	return false;
}

bool AdhocCommandBase::Save(CommandEntryIF* entry)
{
	UNREFERENCED_PARAMETER(entry);

	// 非サポート
	return false;
}

bool AdhocCommandBase::Load(CommandEntryIF* entry)
{
	UNREFERENCED_PARAMETER(entry);

	// 非サポート
	return false;
}

} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp

