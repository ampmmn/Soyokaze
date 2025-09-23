#include "pch.h"
#include "framework.h"
#include "commands/pathconvert/FileProtocolConvertAdhocCommand.h"
#include "commands/pathconvert/Icon.h"
#include "actions/builtin/ExecuteAction.h"
#include "actions/builtin/OpenPathInFilerAction.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "utility/Path.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;
using OpenPathInFilerAction = launcherapp::actions::builtin::OpenPathInFilerAction;
using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;

namespace launcherapp {
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
	Icon mIcon;

	bool mIsEnable{true};
	//
	bool mIsIgnoreUNC{false};
	// 初回呼び出しフラグ(初回呼び出し時に設定をロードするため)
	bool mIsFirstCall{true};
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(FileProtocolConvertAdhocCommand)

FileProtocolConvertAdhocCommand::FileProtocolConvertAdhocCommand() : in(std::make_unique<PImpl>())
{
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
	return _T("⏎:パスをコピー S-⏎:開く C-⏎:フォルダを開く");
}

CString FileProtocolConvertAdhocCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool FileProtocolConvertAdhocCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == Command::MODIFIER_CTRL) {
		// フォルダを開く
		*action = new OpenPathInFilerAction(in->mFullPath);
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_SHIFT) {
		// 開く
		*action = new ExecuteAction(in->mFullPath);
		return true;
	}
	else if (modifierFlags == 0) {
		// パスをコピー
		*action = new CopyTextAction(in->mFullPath);
		return true;
	}
	return false;
}

HICON FileProtocolConvertAdhocCommand::GetIcon()
{
	if (in->mIsIgnoreUNC || Path::FileExists(in->mFullPath) == FALSE) {
		// dummy
		return IconLoader::Get()->LoadUnknownIcon();
	}
	return in->mIcon.Load(in->mFullPath);
}

static void DecodeUri(CString& str)
{
	std::string s;
	UTF2UTF(str, s);

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

	UTF2UTF(dst, str);
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

	in->mIcon.Reset();

	return Pattern::WholeMatch;
}

launcherapp::core::Command*
FileProtocolConvertAdhocCommand::Clone()
{
	auto clonedObj = make_refptr<FileProtocolConvertAdhocCommand>();
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}

CString FileProtocolConvertAdhocCommand::TypeDisplayName()
{
	return _T("パス変換(file://)");
}

} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp


