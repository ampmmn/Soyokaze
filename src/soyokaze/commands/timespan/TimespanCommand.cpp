#include "pch.h"
#include "framework.h"
#include "TimespanCommand.h"
#include "icon/IconLoader.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;

namespace launcherapp {
namespace commands {
namespace timespan {

struct TimespanCommand::PImpl
{
	CTimeSpan mTimeSpan;
	int mUnitType{TYPE_HOUR};
	CString mNum;
	CString mUnit;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(TimespanCommand)

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

CString TimespanCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool TimespanCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	// クリップボードにコピー
	auto modifierFlags = hotkeyAttr.GetModifiers();
	if (modifierFlags == 0) {
		auto a = new actions::clipboard::CopyTextAction(in->mNum);
		a->SetDisplayName(_T("数値のみコピー"));
		*action = a;
		return true;
	}
	else if (modifierFlags == MOD_CONTROL) {
		auto a = new actions::clipboard::CopyTextAction(mName);
		a->SetDisplayName(_T("単位含めてコピー"));
		*action = a;
		return true;
	}
	return false;
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

CString TimespanCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)_T("時間"));
	return TEXT_TYPE;
}

} // end of namespace timespan
} // end of namespace commands
} // end of namespace launcherapp

