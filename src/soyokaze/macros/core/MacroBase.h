#pragma once

#include "macros/core/MacroIF.h"
#include "macros/core/MacroRepository.h"

namespace launcherapp {
namespace macros {
namespace core {

class MacroBase : public MacroIF
{
protected:
	MacroBase();
	virtual ~MacroBase();

public:
	CString GetName() override;
	CString GetDescription() override;
	bool Evaluate(const std::vector<CString>& args, CString& result) override;
	uint32_t AddRef() override;
	uint32_t Release() override;

protected:
	CString mName;
	CString mDescription;
	uint32_t mRefCount{1};

};

}
} // end of namespace core
} // end of namespace launcherapp

