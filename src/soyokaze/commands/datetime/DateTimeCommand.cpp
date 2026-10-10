#include "pch.h"
#include "framework.h"
#include "DateTimeCommand.h"
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
namespace datetime {

struct DateTimeCommand::PImpl
{
	CTimeSpan mTimeSpan;
	int mUnitType{TYPE_HOUR};
	CString mNum;
	CString mUnit;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(DateTimeCommand)

DateTimeCommand::DateTimeCommand(CTimeSpan ts, int unitType) : in(std::make_unique<PImpl>())
{
	in->mTimeSpan = ts;
	in->mUnitType = unitType;

	// 単位ごとに数値と単位文字列を組み立てる
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
	// 候補名と説明は「数値 単位」の形式にする
	this->mName = in->mNum + _T(" ") + in->mUnit;
	this->mDescription = this->mName;
}

DateTimeCommand::~DateTimeCommand()
{
}

CString DateTimeCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool DateTimeCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	// クリップボードにコピー
	auto modifierFlags = hotkeyAttr.GetModifiers();
	if (modifierFlags == 0) {
		// 修飾キーなしは数値のみコピー
		auto a = new actions::clipboard::CopyTextAction(in->mNum);
		a->SetDisplayName(_T("数値のみコピー"));
		*action = a;
		return true;
	}
	else if (modifierFlags == MOD_CONTROL) {
		// Ctrl付きは単位も含めてコピー
		auto a = new actions::clipboard::CopyTextAction(mName);
		a->SetDisplayName(_T("単位含めてコピー"));
		*action = a;
		return true;
	}
	return false;
}


HICON DateTimeCommand::GetIcon()
{
	// 時計のアイコン
	return IconLoader::Get()->GetShell32Icon(-16752);
}

launcherapp::core::Command*
DateTimeCommand::Clone()
{
	return new DateTimeCommand(in->mTimeSpan, in->mUnitType);
}

CString DateTimeCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)_T("時間"));
	return TEXT_TYPE;
}

} // end of namespace datetime
} // end of namespace commands
} // end of namespace launcherapp
