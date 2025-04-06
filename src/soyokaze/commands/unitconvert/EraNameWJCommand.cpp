#include "pch.h"
#include "framework.h"
#include "EraNameWJCommand.h"
#include "commands/common/Clipboard.h"
#include "commands/common/Message.h"
#include "commands/common/CommandParameterFunctions.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace unitconvert {

static std::pair<std::pair<LPCTSTR,LPCTSTR>, int> ERA_TABLE[] = {
	{ {_T("明治"), _T("㍾")}, 1868 },
	{ {_T("大正"), _T("㍽")}, 1912 },
	{ {_T("昭和"), _T("㍼")}, 1926 },
	{ {_T("平成"), _T("㍻")}, 1989 },
	{ {_T("令和"), _T("㋿")}, 2019 },
};

struct EraNameWJCommand::PImpl
{
	void InitMap() {
		for (auto& item : ERA_TABLE) {
			auto eraName1 = item.first.first;
			auto eraName2 = item.first.second;
			auto year = item.second;

			mMapJ2W[eraName1] = year-1;
			mMapJ2W[eraName2] = year-1;

			mMapW2J[year] = eraName1;
		}

	}
	std::map<tstring, int> mMapJ2W;
	std::map<int, tstring> mMapW2J;
	CString mName;
	int mVal = 0;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(EraNameWJCommand)

EraNameWJCommand::EraNameWJCommand() : in(std::make_unique<PImpl>())
{
	in->InitMap();
}

EraNameWJCommand::~EraNameWJCommand()
{
}


CString EraNameWJCommand::GetName()
{
	return in->mName;
}

CString EraNameWJCommand::GetGuideString()
{
	CString str;
	str.Format(_T("Enter:\"%s\"をコピー Shift-Enter:\"%d\"をコピー"), (LPCTSTR)in->mName, in->mVal);
	return str;
}

CString EraNameWJCommand::GetTypeDisplayName()
{
	return _T("元号変換(西暦<->和暦)");
}


BOOL EraNameWJCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// クリップボードにコピー
	uint32_t state = GetModifierKeyState(param, MASK_SHIFT);
	bool isShiftKeyPressed = (state & MASK_SHIFT) != 0;
	if (isShiftKeyPressed == false) {
		Clipboard::Copy(in->mName);
	}
	else {
		CString str;
		str.Format(_T("%d"), in->mVal);
		Clipboard::Copy(str);
	}

	return TRUE;
}

HICON EraNameWJCommand::GetIcon()
{
	return IconLoader::Get()->GetMSSvpIcon(-508);
}

static std::map<tstring, int> MAP_J2W = {
	{ },
};

int EraNameWJCommand::Match(Pattern* pattern)
{
	static tregex patEraJ(_T("(令和|平成|昭和|大正|明治|㍾|㍽|㍼|㍻|㋿) *(([1-9][0-9]*)|元)年"));
	static tregex patEraW(_T("(?:西暦)? *([1-9]\\d{3})年"));
	tstring wholeWord = (tstring)pattern->GetWholeString();

	if (std::regex_match(wholeWord, patEraJ)) {
		// 和暦→西暦
		auto eraName = std::regex_replace(wholeWord, patEraJ, _T("$1"));
		auto yearStr = std::regex_replace(wholeWord, patEraJ, _T("$2"));

		int val = 1;
		if (yearStr != _T("元") && _stscanf_s(yearStr.c_str(), _T("%d"), &val) != 1) {
			return Pattern::Mismatch;
		}

		auto it = in->mMapJ2W.find(eraName);
		if (it == in->mMapJ2W.end()) {
			return Pattern::Mismatch;
		}

		in->mVal = it->second + val;
		in->mName.Format(_T("%d年"), in->mVal);
		return Pattern::FrontMatch;
	}
	else if (std::regex_match(wholeWord, patEraW)) {
		// 西暦→和暦
		auto yearStr = std::regex_replace(wholeWord, patEraW, _T("$1"));
		int val = 1;
		if (_stscanf_s(yearStr.c_str(), _T("%d"), &val) != 1) {
			return Pattern::Mismatch;
		}

		for (auto it = in->mMapW2J.rbegin(); it != in->mMapW2J.rend(); ++it) {
			int th = it->first;
			if (val < th) {
				continue;
			}

			auto eraName = it->second;
			in->mVal = val - th + 1;
			if (in->mVal == 1) {
				in->mName.Format(_T("%s元年"), eraName.c_str());
			}
			else {
				in->mName.Format(_T("%s%d年"), eraName.c_str(), in->mVal);
			}
			return Pattern::FrontMatch;
		}
		return Pattern::Mismatch;
	}
	else {
		return Pattern::Mismatch;
	}
}

launcherapp::core::Command*
EraNameWJCommand::Clone()
{
	auto clonedObj = make_refptr<EraNameWJCommand>();

	clonedObj->mDescription = this->mDescription;
	clonedObj->in->mName = in->mName;

	return clonedObj.release();
}


} // end of namespace unitconvert
} // end of namespace commands
} // end of namespace launcherapp


