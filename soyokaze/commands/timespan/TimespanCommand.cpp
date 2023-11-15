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
};


TimespanCommand::TimespanCommand(CTimeSpan ts, int unitType) : in(std::make_unique<PImpl>())
{
	in->mTimeSpan = ts;
	in->mUnitType = unitType;

	if (unitType == TYPE_HOUR) {
		this->mName.Format(_T("%d時間"), ts.GetTotalHours());
	}
	else if (unitType == TYPE_MINUTE) {
		this->mName.Format(_T("%d分"), ts.GetTotalMinutes());
	}
	else if (unitType == TYPE_SECOND) {
		this->mName.Format(_T("%d秒"), ts.GetTotalSeconds());
	}
	this->mDescription = this->mName;
}

TimespanCommand::~TimespanCommand()
{
}

CString TimespanCommand::GetGuideString()
{
	return _T("Enter:クリップボードにコピー");
}

CString TimespanCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)_T("時間"));
	return TEXT_TYPE;
}

BOOL TimespanCommand::Execute(const Parameter& param)
{
	// クリップボードにコピー
	Clipboard::Copy(mName);
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

