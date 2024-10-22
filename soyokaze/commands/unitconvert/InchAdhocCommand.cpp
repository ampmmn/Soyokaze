#include "pch.h"
#include "framework.h"
#include "InchAdhocCommand.h"
#include "commands/common/Clipboard.h"
#include "commands/common/Message.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using Clipboard = launcherapp::commands::common::Clipboard;


namespace launcherapp {
namespace commands {
namespace unitconvert {

struct InchAdhocCommand::PImpl
{
	CString mName;
};


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
	return _T("Enter:クリップボードにコピー");
}

CString InchAdhocCommand::GetTypeDisplayName()
{
	return _T("単位変換(inch)");
}


BOOL InchAdhocCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// クリップボードにコピー
	Clipboard::Copy(in->mName);
	return TRUE;
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


} // end of namespace unitconvert
} // end of namespace commands
} // end of namespace launcherapp


