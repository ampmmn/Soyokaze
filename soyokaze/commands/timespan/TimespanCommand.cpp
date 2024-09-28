#include "pch.h"
#include "framework.h"
#include "TimespanCommand.h"
#include "icon/IconLoader.h"
#include "commands/common/Clipboard.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace timespan {

constexpr LPCTSTR TYPENAME = _T("TimespanCommand");

struct TimespanCommand::PImpl
{
	CTimeSpan mTimeSpan;
	int mUnitType = TYPE_HOUR;
	CString mNum;
	CString mUnit;
};


TimespanCommand::TimespanCommand(CTimeSpan ts, int unitType) : in(std::make_unique<PImpl>())
{
	in->mTimeSpan = ts;
	in->mUnitType = unitType;

	if (unitType == TYPE_HOUR) {
		in->mNum.Format(_T("%.2f"), ts.GetTotalMinutes() / 60.0);
		in->mUnit = _T("時間");
	}
	else if (unitType == TYPE_MINUTE) {
		in->mNum.Format(_T("%d"), (int)ts.GetTotalMinutes());
		in->mUnit = _T("分");
	}
	else if (unitType == TYPE_SECOND) {
		in->mNum.Format(_T("%d"), (int)ts.GetTotalSeconds());
		in->mUnit = _T("秒");
	}
	this->mName = in->mNum + _T(" ") + in->mUnit;
	this->mDescription = this->mName;
}

TimespanCommand::~TimespanCommand()
{
}

CString TimespanCommand::GetGuideString()
{
	return _T("Enter:数値のみコピー Ctrl-Enter:単位含めてコピー");
}

/**
 * 種別を表す文字列を取得する
 * @return 文字列
 */
CString TimespanCommand::GetTypeName()
{
	return TYPENAME;
}

CString TimespanCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)_T("時間"));
	return TEXT_TYPE;
}

BOOL TimespanCommand::Execute(const Parameter& param)
{
	// クリップボードにコピー
	if (param.GetNamedParamBool(_T("CtrlKeyPressed"))) {
		Clipboard::Copy(mName);
	}
	else {
		Clipboard::Copy(in->mNum);
	}
	return TRUE;
}


HICON TimespanCommand::GetIcon()
{
	return IconLoader::Get()->LoadDefaultIcon();
}

launcherapp::core::Command*
TimespanCommand::Clone()
{
	return new TimespanCommand(in->mTimeSpan, in->mUnitType);
}

} // end of namespace timespan
} // end of namespace commands
} // end of namespace launcherapp

