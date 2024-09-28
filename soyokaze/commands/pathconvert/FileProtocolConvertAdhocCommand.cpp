#include "pch.h"
#include "framework.h"
#include "commands/pathconvert/FileProtocolConvertAdhocCommand.h"
#include "commands/common/Clipboard.h"
#include "commands/common/Message.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "utility/CharConverter.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using Clipboard = launcherapp::commands::common::Clipboard;
using ShellExecCommand = launcherapp::commands::shellexecute::ShellExecCommand;

namespace launcherapp {
namespace commands {
namespace pathconvert {

constexpr LPCTSTR TYPENAME = _T("FileProtocolConvertAdhocCommand");

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
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		reload();
	}
	void OnAppExit() override {}

	void reload()
	{
		auto pref = AppPreference::Get();
		mIsEnable = pref->IsEnableFileProtocolPathConvert();
		mIsIgnoreUNC = pref->IsIgnoreUNC();
	}

	CString mFullPath;
	HICON mIcon = nullptr;

	bool mIsEnable = true;
	//
	bool mIsIgnoreUNC = false;
	// 初回呼び出しフラグ(初回呼び出し時に設定をロードするため)
	bool mIsFirstCall = true;
};


FileProtocolConvertAdhocCommand::FileProtocolConvertAdhocCommand() : in(std::make_unique<PImpl>())
{
}

FileProtocolConvertAdhocCommand::~FileProtocolConvertAdhocCommand()
{
	if (in->mIcon) {
		DestroyIcon(in->mIcon);
		in->mIcon = nullptr;
	}
}


CString FileProtocolConvertAdhocCommand::GetName()
{
	return in->mFullPath;
}

CString FileProtocolConvertAdhocCommand::GetGuideString()
{
	return _T("Enter:パスをコピー Shift-Enter:開く Ctrl-Enter:フォルダを開く");
}

/**
 * 種別を表す文字列を取得する
 * @return 文字列
 */
CString FileProtocolConvertAdhocCommand::GetTypeName()
{
	return TYPENAME;
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
	if (in->mIcon == nullptr) {
		SHFILEINFO sfi = {};
		SHGetFileInfo(in->mFullPath, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON);
		in->mIcon = sfi.hIcon;
	}
	return in->mIcon;
}

static void DecodeUri(CString& str)
{
	launcherapp::utility::CharConverter conv;

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
		sscanf_s(tmp, "%02x", &hex);

		dst.append(1, (char)hex);
		it += 2;
	}

	conv.Convert(dst.c_str(), str);

}

int FileProtocolConvertAdhocCommand::Match(Pattern* pattern)
{
	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		in->reload();
		in->mIsFirstCall = false;
	}

	if (in->mIsEnable == false) {
		return Pattern::Mismatch;
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

	if (in->mIcon) {
		DestroyIcon(in->mIcon);
		in->mIcon = nullptr;
	}

	return Pattern::WholeMatch;
}

launcherapp::core::Command*
FileProtocolConvertAdhocCommand::Clone()
{
	auto clonedObj = std::make_unique<FileProtocolConvertAdhocCommand>();
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}

} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp


