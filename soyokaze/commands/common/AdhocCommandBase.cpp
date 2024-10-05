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
	mDescription(description),
	mRefCount(1)
{
}

AdhocCommandBase::~AdhocCommandBase()
{
}

bool AdhocCommandBase::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	UNREFERENCED_PARAMETER(ifid);
	UNREFERENCED_PARAMETER(cmd);
	// 未実装
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

BOOL AdhocCommandBase::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// 派生側で実装する
	ASSERT(0);
	return TRUE;
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

bool AdhocCommandBase::IsEditable()
{
	return false;
}

bool AdhocCommandBase::IsDeletable()
{
	return false;
}

int AdhocCommandBase::EditDialog(HWND parent)
{
	UNREFERENCED_PARAMETER(parent);

	return -1;
}

bool AdhocCommandBase::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	UNREFERENCED_PARAMETER(attr);

	// 一時コマンドはホットキー設定を持たない
	return false;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool AdhocCommandBase::IsPriorityRankEnabled()
{
	// 基本は重みづけをする
	return true;
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

uint32_t AdhocCommandBase::AddRef()
{
	return ++mRefCount;
}

uint32_t AdhocCommandBase::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp

