#pragma once

#include "macros/core/MacroBase.h"

namespace launcherapp {
namespace macros {
namespace builtin {

class DateMacro : public launcherapp::macros::core::MacroBase
{
	DateMacro();
	~DateMacro() override;

public:
	bool Evaluate(const std::vector<CString>& args, std::vector<CString>& results) override;

	static CTimeSpan MakeTimeSpan(const CString& str);
	static int ComputeDayFromYear(int offset, CTime tmToday);
	static int ComputeDayFromMonth(int offset, CTime tmToday);
	DECLARE_LAUNCHERMACRO(DateMacro)
};


}
}
}
