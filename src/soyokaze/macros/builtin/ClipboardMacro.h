#pragma once

#include "macros/core/MacroBase.h"

namespace launcherapp {
namespace macros {
namespace builtin {

class ClipboardMacro : public launcherapp::macros::core::MacroBase
{
	ClipboardMacro();
	virtual ~ClipboardMacro();

public:
	bool Evaluate(const std::vector<CString>& args, CString& result) override;

	DECLARE_LAUNCHERMACRO(ClipboardMacro)
};


}
}
}
