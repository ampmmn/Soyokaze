#include "pch.h"
#include "MacroBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif	

namespace launcherapp {
namespace macros {
namespace core {


MacroBase::MacroBase()
{
}

MacroBase::~MacroBase()
{
}

CString MacroBase::GetName()
{
	return mName;
}

CString MacroBase::GetDescription()
{
	return mDescription;
}

bool MacroBase::Evaluate(const std::vector<CString>& args, CString& result)
{
	UNREFERENCED_PARAMETER(args);
	UNREFERENCED_PARAMETER(result);

	spdlog::warn(_T("Macro has no implemetation. name:{}"), (LPCTSTR)GetName());
	return false;
}

uint32_t MacroBase::AddRef()
{
	return ++mRefCount;
}

uint32_t MacroBase::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}


}
}
}