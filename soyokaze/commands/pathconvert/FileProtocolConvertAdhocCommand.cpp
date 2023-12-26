#include "pch.h"
#include "framework.h"
#include "commands/pathconvert/FileProtocolConvertAdhocCommand.h"
#include "commands/common/Clipboard.h"
#include "commands/common/Message.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "utility/CharConverter.h"
#include "AppPreferenceListenerIF.h"
#include "AppPreference.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using Clipboard = soyokaze::commands::common::Clipboard;
using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;

namespace soyokaze {
namespace commands {
namespace pathconvert {

struct FileProtocolConvertAdhocCommand::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void OnAppFirstBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mIsIgnoreUNC = pref->IsIgnoreUNC();
	}
	void OnAppExit() override {}

	CString mFullPath;
	//
	bool mIsIgnoreUNC;
	// 初回呼び出しフラグ(初回呼び出し時に設定をロードするため)
	bool mIsFirstCall;
};


FileProtocolConvertAdhocCommand::FileProtocolConvertAdhocCommand() : in(std::make_unique<PImpl>())
{
	in->mIsIgnoreUNC = false;
	in->mIsFirstCall = true;
}

FileProtocolConvertAdhocCommand::~FileProtocolConvertAdhocCommand()
{
}


CString FileProtocolConvertAdhocCommand::GetName()
{
	return in->mFullPath;
}

CString FileProtocolConvertAdhocCommand::GetGuideString()
{
	return _T("Enter:パスをコピー Shift-Enter:開く Ctrl-Enter:フォルダを開く");
}

CString FileProtocolConvertAdhocCommand::GetTypeDisplayName()
{
	return _T("パス変換(file://)");
}

BOOL FileProtocolConvertAdhocCommand::Execute(const Parameter& param)
{
	bool isCtrlPressed = param.GetNamedParamBool(_T("CtrlKeyPressed"));
	bool isShiftPressed = param.GetNamedParamBool(_T("ShiftKeyPressed"));
	if (isCtrlPressed != false || isShiftPressed != false) {
		// フォルダを開く or 開く
		ShellExecCommand cmd;
		cmd.SetPath(in->mFullPath);
		return cmd.Execute(param);
	}
	else {
		// クリップボードにコピー
		Clipboard::Copy(in->mFullPath);
	}
	return TRUE;
}

HICON FileProtocolConvertAdhocCommand::GetIcon()
{
	if (in->mIsIgnoreUNC || PathFileExists(in->mFullPath) == FALSE) {
		// dummy
		return IconLoader::Get()->LoadUnknownIcon();
	}

	SHFILEINFO sfi = {};
	HIMAGELIST hImgList =
		(HIMAGELIST)::SHGetFileInfo(in->mFullPath, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON);
	HICON hIcon = sfi.hIcon;
	return hIcon;
}

static void DecodeUri(CString& str)
{
	soyokaze::utility::CharConverter conv;

	std::string s;
	conv.Convert(str, s);

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
	}

	conv.Convert(dst.c_str(), str);

}

int FileProtocolConvertAdhocCommand::Match(Pattern* pattern)
{
	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		auto pref = AppPreference::Get();
		in->mIsIgnoreUNC = pref->IsIgnoreUNC();
		in->mIsFirstCall = false;
	}

	CString wholeWord = pattern->GetWholeString();

	// file://ではじまるものか判断する
	static tregex patProtocol(_T("^ *file://.+"));
	if (std::regex_match(tstring(wholeWord), patProtocol) == false) {
		return Pattern::Mismatch;
	}

	// file://～ のパスを取り出す(file://を除去する)
	static tregex patReplace(_T("^ *file://(.+) *$"));
	auto pathBody = std::regex_replace(tstring(wholeWord), patReplace, _T("$1"));

	static tregex pat1(_T("^//.*"));
	static tregex pat2(_T("^/[^/].*"));
	if (std::regex_match(tstring(pathBody), pat1)) {
		// file:// を除いた後の先頭が//なら、そのまま(UNCパスとして扱う)
		in->mFullPath = pathBody.c_str();
	}
	else if (std::regex_match(tstring(pathBody), pat2)) {
		// file:// を除いた後の先頭が/なら、(おそらくローカルパスの絶対パス表記なので)先頭の/だけ除外
		// 先頭一文字を除外
		in->mFullPath = &(*(pathBody.begin() + 1));
	}
	else {
		// file:// を除いた後の先頭が/でないなら、(おそらくUNCパスになるので)先頭に\\を付与
		in->mFullPath = _T("\\\\");
		in->mFullPath += pathBody.c_str();
	}

	// 区切り文字をバックスラッシュに変換する
	in->mFullPath.Replace(_T('/'), _T('\\'));

	DecodeUri(in->mFullPath);

	return Pattern::WholeMatch;
}

soyokaze::core::Command*
FileProtocolConvertAdhocCommand::Clone()
{
	auto clonedObj = std::make_unique<FileProtocolConvertAdhocCommand>();
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}

} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace soyokaze


