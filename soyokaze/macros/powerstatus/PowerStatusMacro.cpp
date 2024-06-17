#include "pch.h"
#include "PowerStatusMacro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace macros {
namespace powerstatus {

REGISTER_LAUNCHERMACRO(PowerStatusMacro)

PowerStatusMacro::PowerStatusMacro()
{
	mName = _T("powerstatus");
}

PowerStatusMacro::~PowerStatusMacro()
{
}

bool PowerStatusMacro::Evaluate(const std::vector<CString>& args, CString& result)
{
	if (args.size() == 0) {
		return false;
	}

	SYSTEM_POWER_STATUS sts;
	GetSystemPowerStatus(&sts);

	CString fmt = args[0];

	int n = fmt.Find(_T("%l"));
	if (n != -1) {
		BYTE life = sts.BatteryLifePercent;
		CString val;
		if (life < 100) {
			val.Format(_T("%d%%"), (int)life);
		}
		else if (life == 255) {
			val = _T("(不明)");
		}
		fmt.Replace(_T("%l"), val);
	}
	n = fmt.Find(_T("%a"));
	if (n != -1) {
		BYTE acStatus = sts.ACLineStatus;
		CString val;
		if (acStatus == 1) {
			val = _T("オンライン");
		}
		else if (acStatus == 0) {
			val = _T("オフライン");
		}
		else {
			val = _T("不明");
		}
		fmt.Replace(_T("%a"), val);
	}
	result = fmt;
	return true;
}


}
}
}
