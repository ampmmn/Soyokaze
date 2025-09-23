#include "pch.h"
#include "framework.h"
#include "EscapedCharCommand.h"
#include "icon/IconLoader.h"
#include "commands/common/Clipboard.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "utility/CharConverter.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

using CharConverter = launcherapp::utility::CharConverter;
using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;

namespace launcherapp {
namespace commands {
namespace decodestring {

struct EscapedCharCommand::PImpl
{
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(EscapedCharCommand)

EscapedCharCommand::EscapedCharCommand() : in(std::make_unique<PImpl>())
{
}

EscapedCharCommand::~EscapedCharCommand()
{
}

CString EscapedCharCommand::GetName()
{
	auto name = mName;
	name.Replace(_T("\r\n"), _T(""));
	name.Replace(_T("\n"), _T(""));
	return name;
}

CString EscapedCharCommand::GetDescription()
{
	auto name = mName;
	name.Replace(_T("\r\n"), _T(""));
	name.Replace(_T("\n"), _T(""));
	return name;
}

CString EscapedCharCommand::GetGuideString()
{
	return _T("⏎:デコード後の文字列をコピー");
}

CString EscapedCharCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool EscapedCharCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags != 0) {
		return false;
	}

	// クリップボードにコピー
	*action = new CopyTextAction(mName);
	return true;
}


HICON EscapedCharCommand::GetIcon()
{
	return IconLoader::Get()->LoadConvertIcon();
}

bool EscapedCharCommand::ScanAsU4(std::string::iterator& it, std::string::iterator itEnd, std::string& dst)
{
	// この時点で \u までは一致してることを確認済、itは'\'の位置にいる
	if (std::distance(it, itEnd) < 6) {
		// 残り文字数がたりない
		return false;
	}

	if (std::any_of(it+2, it+6, [](auto c) { return isxdigit(c) == 0; })) {
		// 条件を満たさない(後続4文字が16進文字でない)
		return false;
	}

	wchar_t wc[2];

	char tmp[] = { *(it+2), *(it+3), *(it+4), *(it+5), '\0' };
	int n = sscanf_s(tmp, "%04hx", (unsigned short*)wc);
	ASSERT(n == 1);

	bool isSurrogateTrailingChar = (0xD800 <= wc[0] && wc[0] <= 0xDFFF);
	if (!isSurrogateTrailingChar) {
		// サロゲートペア文字ではない
		char out[5] = {};
		int converted = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wc, 1, out, 5, 0, 0);
		if (converted == 0) {
			return false;
		}

		dst.append(out, out + converted);
		it += 6;
		return true;
	}

	// サロゲートペア文字だったので後続の文字も解析したうえで、ペアとして扱う

	auto it2 = it + 6;  // 後続文字を解析位置をあらわすイテレータ
	if (std::distance(it2, itEnd) < 6) {
		// 残り文字数がたりない
		return false;
	}

	if (*it2 != '\\' || *(it2+1) != 'u' ||
	    std::any_of(it2 + 2, it2 + 6, [](auto c) { return isxdigit(c) == 0; })) {
		// 条件を満たさない(後続4文字が16進文字でない)
		return false;
	}

	char tmp2[] = { *(it2+2), *(it2+3), *(it2+4), *(it2+5), '\0' };
	n = sscanf_s(tmp2, "%04hx", (unsigned short*)(wc + 1));
	ASSERT(n == 1);

	char out[16] = {};
	int converted = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wc, 2, out, 16, 0, 0);
	if (converted == 0) {
		return false;
	}

	dst.append(out, out + converted);
	it += 12;

	return true;
}

bool EscapedCharCommand::ScanAsU8(std::string::iterator& it, std::string::iterator itEnd, std::string& dst)
{
	// この時点で \U までは一致してることを確認済、itは'\'の位置にいる
	if (std::distance(it, itEnd) < 10) {
		// 残り文字数がたりない
		return false;
	}

	if (isxdigit(*(it+2)) == 0 || isxdigit(*(it+3)) == 0 ||
      isxdigit(*(it+4)) == 0 || isxdigit(*(it+5)) == 0 ||
      isxdigit(*(it+6)) == 0 || isxdigit(*(it+7)) == 0 ||
      isxdigit(*(it+8)) == 0 || isxdigit(*(it+9)) == 0) {
		// 条件を満たさない(後続8文字が16進文字でない)
		return false;
	}

	uint32_t scalar;
	char tmp[] = { *(it+2), *(it+3), *(it+4), *(it+5), *(it+6), *(it+7), *(it+8), *(it+9), '\0' };
	sscanf_s(tmp, "%08x", &scalar);

	// スカラ値をutf-8表現に変換
	char out[5] = {};
	int converted = CharConverter::ScalarToUTF8(scalar, out);

	dst.append(out, out + converted);
	it += 10;

	return true;
}

bool EscapedCharCommand::ScanAsHex(std::string::iterator& it, std::string::iterator itEnd, std::string& dst)
{
	// この時点で \x までは一致してることを確認済、itは'\'の位置にいる
	if (std::distance(it, itEnd) < 4) {
		// 残り文字数がたりない
		return false;
	}
	if (isxdigit(*(it+2)) == 0 || isxdigit(*(it+3)) == 0) {
		// 条件を満たさない(後続2文字が16進文字でない)
		return false;
	}

	char chr;

	char tmp[] = { *(it+2), *(it+3), '\0' };
	sscanf_s(tmp, "%hhx", &chr);

	dst.append(1, chr);
	it += 4;

	return true;
}

bool EscapedCharCommand::ScanAsOctal(std::string::iterator& it, std::string::iterator itEnd, std::string& dst)
{
	// この時点で \o までは一致してることを確認済、itは'\'の位置にいる
	if (std::distance(it, itEnd) < 4) {
		// 残り文字数がたりない
		return false;
	}

	auto isoctal = [](char c) { return '0' <= c && c <= '7'; };
	if (isoctal(*(it+1)) == false || isoctal(*(it+2)) == false || isoctal(*(it+3)) == false) {
		// 条件を満たさない(後続3文字が8進文字でない)
		return false;
	}

	char chr;

	char tmp[] = { *(it+1), *(it+2), *(it+3), '\0' };
	sscanf_s(tmp, "%hho", &chr);

	dst.append(1, chr);
	it += 4;

	return true;
}

int EscapedCharCommand::Match(Pattern* pattern)
{
	CString cmdline = pattern->GetWholeString();

	std::string s;
	UTF2UTF(cmdline, s);

	bool isMatched = false;

	std::string dst;
	for (auto it = s.begin(); it != s.end(); ++it) {

		if (*it != '\\') {
			dst.append(1, *it);
			continue;
	 	}

		if (it+1 == s.end()) {
			dst.append(1, *it);
			break;
		}

		if (*(it+1) == 'u') {
			// \uXXXX
			if (ScanAsU4(it, s.end(), dst) == false) {
				dst.append(it, it + 2);
				it ++;
				continue;
			}

			// ScanAsXXXのなかで次の文字のイテレータに移動するが、forのstep処理でインクメントされるため一つ戻す
			it--;

			isMatched = true;
			continue;
		}
		else if (*(it+1) == 'U') {
			// \UXXXXXXXX
			if (ScanAsU8(it, s.end(), dst) == false) {
				dst.append(it, it + 2);
				it++;
				continue;
			}
			// ScanAsXXXのなかで次の文字のイテレータに移動するが、forのstep処理でインクメントされるため一つ戻す
			it--;

			isMatched = true;
			continue;
		}
		else if (*(it+1) == 'x') {
			// \xXX
			if (ScanAsHex(it, s.end(), dst) == false) {
				dst.append(it, it + 2);
				it++;
				continue;
			}

			// ScanAsXXXのなかで次の文字のイテレータに移動するが、forのstep処理でインクメントされるため一つ戻す
			it--;

			isMatched = true;
			continue;
		}
		else if (_istdigit(*(it+1))) {
			// \ooo
			if (ScanAsOctal(it, s.end(), dst) == false) {
				dst.append(it, it + 2);
				it++;
				continue;
			}

			// ScanAsXXXのなかで次の文字のイテレータに移動するが、forのstep処理でインクメントされるため一つ戻す
			it--;

			isMatched = true;
			continue;
		}
		else {
			dst.append(1, *it);
			continue;
		}
	}

	if (isMatched == false) {
		return Pattern::Mismatch;
	}

	UTF2UTF(dst, mName);

	return Pattern::PartialMatch;
}

launcherapp::core::Command*
EscapedCharCommand::Clone()
{
	return new EscapedCharCommand();
}

CString EscapedCharCommand::TypeDisplayName()
{
	static CString TEXT_TYPE(_T("EscapedChar"));
	return TEXT_TYPE;
}

} // end of namespace decodestring
} // end of namespace commands
} // end of namespace launcherapp

