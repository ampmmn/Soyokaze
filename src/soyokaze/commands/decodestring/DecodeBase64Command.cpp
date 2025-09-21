#include "pch.h"
#include "framework.h"
#include "DecodeBase64Command.h"
#include "icon/IconLoader.h"
#include "commands/common/Clipboard.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "utility/CharConverter.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;

namespace launcherapp {
namespace commands {
namespace decodestring {

constexpr std::array<uint8_t, 256> DECODE_TABLE{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x3E, 0x00, 0x3F,   // +- / 
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0-9
	0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,   // A-
	0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x3F,   // -Z _
	0x00, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,   // a-
	0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00,   // -z
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(DecodeBase64Command)

DecodeBase64Command::DecodeBase64Command()
{
}

DecodeBase64Command::~DecodeBase64Command()
{
}

CString DecodeBase64Command::GetName()
{
	auto name = mName;
	name.Replace(_T("\r\n"), _T(""));
	name.Replace(_T("\n"), _T(""));
	return name;
}

CString DecodeBase64Command::GetDescription()
{
	auto name = mName;
	name.Replace(_T("\r\n"), _T(""));
	name.Replace(_T("\n"), _T(""));
	return name;
}

CString DecodeBase64Command::GetGuideString()
{
	return _T("⏎:デコード後データをコピー");
}


CString DecodeBase64Command::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool DecodeBase64Command::GetAction(uint32_t modifierFlags, Action** action)
{
	UNREFERENCED_PARAMETER(modifierFlags);
	// クリップボードにコピー
	*action = new CopyTextAction(mName);
	return true;
}


HICON DecodeBase64Command::GetIcon()
{
	return IconLoader::Get()->LoadConvertIcon();
}

int DecodeBase64Command::Match(Pattern* pattern)
{
	CString cmdline = pattern->GetWholeString();

	launcherapp::utility::CharConverter conv;

	std::string s;
	conv.Convert(cmdline, s);

	if (s.size() < 16) {
		// あまり短いものを変換すると、たまたまBase64に合致するワードも変換してしまうため除外
		return Pattern::Mismatch;
	}

	// 検証
	for (auto it = s.begin(); it != s.end(); ++it) {
		uint8_t c = (uint8_t)*it;

		bool isPadding = c == '=';
		if (isPadding) {
			break;
		}

		bool isValid = ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9') || 
		               c == '+' || c == '/' || c == '-' || c == '_';   // '-'と'_'もBase64URL Encodingとして使われるため、対象に含める
		if (isValid == false) {
			return Pattern::Mismatch;
		}
	}

	struct Bits {
		void flush(std::vector<uint8_t>& dst) {
			
			if (index == 0 || index == 1) {
				return;
			}
			// 端数(1byte or 2byte)分を変換する
			int n = (index == 3) ? 2 : 1;

			char s[3];
			s[0] = ((bits[0] << 2) & 0xFC) | ((bits[1] >> 4) & 0x03);
			if (index == 3) {
				s[1] = ((bits[1] << 4) & 0xF0) | ((bits[2] >> 2) & 0x0F);
			}
			dst.insert(dst.end(), s, s + n);
			index = 0;
	 	}
		void add(uint8_t c, std::vector<uint8_t>& dst) {
			bits[index++] = DECODE_TABLE[c];
			if (index == 4) {
				char s[3];
				s[0] = ((bits[0] << 2) & 0xFC) | ((bits[1] >> 4) & 0x03);
				s[1] = ((bits[1] << 4) & 0xF0) | ((bits[2] >> 2) & 0x0F);
				s[2] = ((bits[2] << 6) & 0xC0) | ((bits[3] >> 0) & 0x3F);
				dst.insert(dst.end(), s, s + 3);
				index = 0;
			}
		}
		uint8_t bits[4] = {};
		int index = 0;
	} bits;

	std::vector<uint8_t> dst;
	for (auto it = s.begin(); it != s.end(); ++it) {

		uint8_t c = (uint8_t)*it;

//	'A': 0x41
//	'Z': 0x5a
//	'a': 0x61
//	'z': 0x7a
//	'0': 0x30
//	'9': 0x39
//	'+': 0x2b
//	'/': 0x2f

		bool isPadding = c == '=';
		if (isPadding) {
			break;
		}
		bits.add(c, dst);
	}
	bits.flush(dst);
	dst.push_back(0x00);

	try {
		// Base64デコードして得られたバイト列をUTF-8とみなしてwchar_t配列に変換する
		bool isFailInvalidChars = true;
		conv.Convert((const char*)dst.data(), mName, isFailInvalidChars);
		return Pattern::PartialMatch;
	}
	catch(launcherapp::utility::CharConverter::Exception) {
		// UTF-8文字列として変換できない場合は不一致扱いとする
		return Pattern::Mismatch;
	}
	return Pattern::Mismatch;
}

launcherapp::core::Command*
DecodeBase64Command::Clone()
{
	return new DecodeBase64Command();
}

CString DecodeBase64Command::TypeDisplayName()
{
	static CString TEXT_TYPE(_T("DecodeBase64"));
	return TEXT_TYPE;
}

} // end of namespace decodestring
} // end of namespace commands
} // end of namespace launcherapp

