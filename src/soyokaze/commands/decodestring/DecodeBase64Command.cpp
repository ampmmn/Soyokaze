#include "pch.h"
#include "framework.h"
#include "DecodeBase64Command.h"
#include "icon/IconLoader.h"
#include "commands/common/Clipboard.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "utility/CharConverter.h"
#include "resource.h"
#include <simdutf.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;

namespace launcherapp {
namespace commands {
namespace decodestring {

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


CString DecodeBase64Command::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool DecodeBase64Command::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() != 0) {
		return false;
	}
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

	for (char c : s) {
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f') {
			return Pattern::Mismatch;
		}
	}

	std::vector<char> dst(simdutf::maximal_binary_length_from_base64(s.data(), s.size()));
	auto result = simdutf::base64_to_binary(
		s.data(), s.size(), dst.data(), simdutf::base64_default_or_url);
	if (result.error != simdutf::error_code::SUCCESS) {
		return Pattern::Mismatch;
	}
	dst.resize(result.count);
	dst.push_back('\0');

	try {
		// Base64デコードして得られたバイト列をUTF-8とみなしてwchar_t配列に変換する
		bool isFailInvalidChars = true;
		conv.Convert(dst.data(), mName, isFailInvalidChars);
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
