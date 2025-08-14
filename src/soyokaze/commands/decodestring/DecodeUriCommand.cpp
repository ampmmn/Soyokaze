#include "pch.h"
#include "framework.h"
#include "DecodeUriCommand.h"
#include "icon/IconLoader.h"
#include "commands/common/Clipboard.h"
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

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(DecodeUriCommand)

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
	return _T("⏎:デコード後の文字列をコピー");
}

CString DecodeUriCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

BOOL DecodeUriCommand::Execute(Parameter* param)
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

	std::string s;
	UTF2UTF(cmdline, s);

	return DecodeURI(s, mName) ? Pattern::PartialMatch : Pattern::Mismatch;
}

launcherapp::core::Command*
DecodeUriCommand::Clone()
{
	return new DecodeUriCommand();
}

CString DecodeUriCommand::TypeDisplayName()
{
	static CString TEXT_TYPE(_T("DecodeURI"));
	return TEXT_TYPE;
}

bool DecodeUriCommand::DecodeURI(std::string& src, CString& decoded)
{
	// static std::regex reg("^.*%[0-9a-fA-F][0-9a-fA-F].*$");
	// if (std::regex_match(s, reg) == false) {
	// 	return Pattern::Mismatch;
	// }

	bool isMatched = false;

	std::string dst;
	for (auto it = src.begin(); it != src.end(); ++it) {

		if (*it != '%') {
			dst.append(1, *it);
			continue;
	 	}

		if (it+1 == src.end()) {
			dst.append(1, *it);
			break;
		}
		if (_istxdigit(*(it+1)) == 0) {
			dst.append(it, it + 2);
			it++;
			continue;
		}

		if (it+2 == src.end()) {
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
		return false;
	}

	UTF2UTF(dst, decoded);
	return true;
}

} // end of namespace decodestring
} // end of namespace commands
} // end of namespace launcherapp

