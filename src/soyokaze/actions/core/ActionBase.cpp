#include "pch.h"
#include "ActionBase.h"
#include "core/IFIDDefine.h"

namespace launcherapp { namespace actions { namespace core {


ActionBase::ActionBase()
{
}

ActionBase::~ActionBase()
{
}

void ActionBase::SetVisible(bool isVisible)
{
	mIsVisible = isVisible;
}

// アクションの内容を示す名称
CString ActionBase::GetDisplayName()
{
	spdlog::error("ActionBase::GetDisplayName called!");
	return _T("(未実装)");
}

// アクションを実行する
bool ActionBase::Perform(Parameter* param, String* errMsg)
{
	UNREFERENCED_PARAMETER(param);

	if (errMsg) {
		*errMsg = "(Performメソッド未実装)";
	}
	spdlog::error("ActionBase::Perform called!");
	return false;
}

// ガイド欄などに表示するかどうか
bool ActionBase::IsVisible()
{
	return mIsVisible;
}

bool ActionBase::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (ifid == IFID_ACTION) {
		*cmd = (void*)this;
		return true;
	}
	spdlog::error("ActionBase::QueryInterface called!");
	return false;
}
uint32_t ActionBase::AddRef()
{
	return (uint32_t)InterlockedIncrement(&mRefCount);
}

uint32_t ActionBase::Release()
{
	auto n = InterlockedDecrement(&mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}

}}}

