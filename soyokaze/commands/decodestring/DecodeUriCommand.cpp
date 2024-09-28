#include "pch.h"
#include "framework.h"
#include "DecodeUriCommand.h"
#include "icon/IconLoader.h"
#include "commands/common/Clipboard.h"
#include "utility/CharConverter.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace decodestring {

constexpr LPCTSTR TYPENAME = _T("DecodeUriCommand");

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
	return _T("Enter:デコード後の文字列をコピー");
}

/**
 * 種別を表す文字列を取得する
 * @return 文字列
 */
CString DecodeUriCommand::GetTypeName()
{
	return TYPENAME;
}

CString DecodeUriCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE(_T("DecodeURI"));
	return TEXT_TYPE;
}

BOOL DecodeUriCommand::Execute(const Parameter& param)
{
	UNREFERENCED_PARAMETER(param);

	// クリップボードにコピー
	Clipboard::Copy(mName);
	return TRUE;
}


HICON DecodeUriCommand::GetIcon()
{
	return IconLoader::Get()->LoadConvertIcon();
}

int DecodeUriCommand::Match(Pattern* pattern)
{
	CString cmdline = pattern->GetWholeString();

	launcherapp::utility::CharConverter conv;

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
		sscanf_s(tmp, "%02x", &hex);

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

launcherapp::core::Command*
DecodeUriCommand::Clone()
{
	return new DecodeUriCommand();
}

} // end of namespace decodestring
} // end of namespace commands
} // end of namespace launcherapp

