#include "pch.h"
#include "framework.h"
#include "TimespanCommand.h"
#include "IconLoader.h"
#include "commands/common/Clipboard.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace soyokaze::commands::common;

namespace soyokaze {
namespace commands {
namespace timespan {


struct TimespanCommand::PImpl
{
	CTimeSpan mTimeSpan;
	int mUnitType;
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
		in->mNum.Format(_T("%d"), ts.GetTotalMinutes());
		in->mUnit = _T("分");
	}
	else if (unitType == TYPE_SECOND) {
		in->mNum.Format(_T("%d"), ts.GetTotalSeconds());
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

soyokaze::core::Command*
TimespanCommand::Clone()
{
	return new TimespanCommand(in->mTimeSpan, in->mUnitType);
}

} // end of namespace timespan
} // end of namespace commands
} // end of namespace soyokaze

