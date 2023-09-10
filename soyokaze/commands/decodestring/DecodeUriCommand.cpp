#include "pch.h"
#include "framework.h"
#include "DecodeUriCommand.h"
#include "IconLoader.h"
#include "commands/common/Clipboard.h"
#include "utility/CharConverter.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace soyokaze::commands::common;

namespace soyokaze {
namespace commands {
namespace decodestring {


struct DecodeUriCommand::PImpl
{
};


DecodeUriCommand::DecodeUriCommand() : in(std::make_unique<PImpl>())
{
}

DecodeUriCommand::~DecodeUriCommand()
{
}

CString DecodeUriCommand::GetName()
{
	auto name = mName;
	name.Replace(_T("\r\n"), _T(""));
	name.Replace(_T("\n"), _T(""));
	return name;
}

CString DecodeUriCommand::GetDescription()
{
	auto name = mName;
	name.Replace(_T("\r\n"), _T(""));
	name.Replace(_T("\n"), _T(""));
	return name;
}

CString DecodeUriCommand::GetGuideString()
{
	return _T("Enter:クリップボードにコピー");
}


CString DecodeUriCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE(_T("DecodeURI"));
	return TEXT_TYPE;
}

BOOL DecodeUriCommand::Execute(const Parameter& param)
{
	// クリップボードにコピー
	Clipboard::Copy(mName);
	return TRUE;
}


HICON DecodeUriCommand::GetIcon()
{
	// ToDo: 設定
	return IconLoader::Get()->LoadDefaultIcon();
}

int DecodeUriCommand::Match(Pattern* pattern)
{
	CString cmdline = pattern->GetWholeString();

	soyokaze::utility::CharConverter conv;

	std::string s;
	conv.Convert(cmdline, s);

	// static std::regex reg("^.*%[0-9a-fA-F][0-9a-fA-F].*$");
	// if (std::regex_match(s, reg) == false) {
	// 	return Pattern::Mismatch;
	// }

	bool isMatched = false;

	std::string dst;
	for (auto it = s.begin(); it != s.end(); ++it) {

		if (*it != '%') {
			dst.append(1, *it);
			continue;
	 	}

		if (it+1 == s.end()) {
			dst.append(1, *it);
			break;
		}
		if (_istxdigit(*(it+1)) == 0) {
			dst.append(it, it + 2);
			it++;
			continue;
		}

		if (it+2 == s.end()) {
			dst.append(it, it + 2);
			break;
		}
		if (_istxdigit(*(it+2)) == 0) {
			dst.append(it, it + 3);
			it+=2;
			continue;
		}

		uint32_t hex;

		char tmp[] = { *(it+1), *(it+2), '\0' };
		int n = sscanf_s(tmp, "%02x", &hex);
		ASSERT(n == 1);

		dst.append(1, (char)hex);
		it += 2;

		isMatched = true;
	}

	if (isMatched == false) {
		return Pattern::Mismatch;
	}

	conv.Convert(dst.c_str(), mName);

	return Pattern::PartialMatch;
}

soyokaze::core::Command*
DecodeUriCommand::Clone()
{
	return new DecodeUriCommand();
}

} // end of namespace decodestring
} // end of namespace commands
} // end of namespace soyokaze

