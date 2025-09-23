#include "pch.h"
#include "framework.h"
#include "InchAdhocCommand.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "icon/IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;


namespace launcherapp {
namespace commands {
namespace unitconvert {

struct InchAdhocCommand::PImpl
{
	CString mName;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(InchAdhocCommand)

InchAdhocCommand::InchAdhocCommand() : in(std::make_unique<PImpl>())
{
}

InchAdhocCommand::~InchAdhocCommand()
{
}


CString InchAdhocCommand::GetName()
{
	return in->mName;
}

CString InchAdhocCommand::GetGuideString()
{
	return _T("⏎:クリップボードにコピー");
}

CString InchAdhocCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}


bool InchAdhocCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags != 0) {
		return false;
	}
	// クリップボードにコピー
	*action = new actions::clipboard::CopyTextAction(in->mName);
	return true;
}

HICON InchAdhocCommand::GetIcon()
{
	// dummy
	return IconLoader::Get()->LoadUnknownIcon();
}

int InchAdhocCommand::Match(Pattern* pattern)
{
	tstring wholeWord = (tstring)pattern->GetWholeString();

	static tregex patInch(_T("^ *([0-9.]+) *inch$"));
	static tregex patMM(_T("^ *([0-9.]+) *mm$"));
	if (std::regex_match(wholeWord, patInch)) {
		auto valStr = std::regex_replace(wholeWord, patInch, _T("$1"));
		double val;
		if (_stscanf_s(valStr.c_str(), _T("%lf"), &val) != 1) {
			return Pattern::Mismatch;
		}

		in->mName.Format(_T("%g mm"), val * 25.4);
		return Pattern::PartialMatch;
	}
	else if (std::regex_match(wholeWord, patMM)) {
		auto valStr = std::regex_replace(wholeWord, patMM, _T("$1"));
		double val;
		if (_stscanf_s(valStr.c_str(), _T("%lf"), &val) != 1) {
			return Pattern::Mismatch;
		}

		in->mName.Format(_T("%g inch"), val / 25.4);
		return Pattern::PartialMatch;
	}
	else {
		return Pattern::Mismatch;
	}
}

launcherapp::core::Command*
InchAdhocCommand::Clone()
{
	auto clonedObj = make_refptr<InchAdhocCommand>();

	clonedObj->mDescription = this->mDescription;
	clonedObj->in->mName = in->mName;

	return clonedObj.release();
}

CString InchAdhocCommand::TypeDisplayName()
{
	return _T("単位変換(inch)");
}

} // end of namespace unitconvert
} // end of namespace commands
} // end of namespace launcherapp


