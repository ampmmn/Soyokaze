#include "pch.h"
#include "ActionBase.h"
#include "core/IFIDDefine.h"

namespace launcherapp { namespace actions { namespace core {

// アクションの内容を示す名称
CString ActionBase::GetDisplayName()
{
	spdlog::error("ActionBase::GetDisplayName called!");
	return _T("(未実装)");
}

// アクションを実行する
bool ActionBase::Perform(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	ASSERT(0);
	spdlog::error("ActionBase::Perform called!");
	return false;
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

