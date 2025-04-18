#include "pch.h"
#include "framework.h"
#include "commands/pathconvert/LocalToGitBashPathAdhocCommand.h"
#include "commands/common/Clipboard.h"
#include "commands/common/Message.h"
#include "icon/IconLoader.h"
#include "utility/Path.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


using Clipboard = launcherapp::commands::common::Clipboard;

namespace launcherapp {
namespace commands {
namespace pathconvert {

struct LocalToGitBashPathAdhocCommand::PImpl
{
	CString mFullPath;
	HICON mIcon{nullptr};
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(LocalToGitBashPathAdhocCommand)

LocalToGitBashPathAdhocCommand::LocalToGitBashPathAdhocCommand() : in(std::make_unique<PImpl>())
{
}

LocalToGitBashPathAdhocCommand::~LocalToGitBashPathAdhocCommand()
{
	if (in->mIcon) {
		DestroyIcon(in->mIcon);
		in->mIcon = nullptr;
	}
}


CString LocalToGitBashPathAdhocCommand::GetName()
{
	return in->mFullPath;
}

CString LocalToGitBashPathAdhocCommand::GetGuideString()
{
	return _T("Enter:パスをコピー");
}

CString LocalToGitBashPathAdhocCommand::GetTypeDisplayName()
{
	return _T("Local To git-bash Path");
}

BOOL LocalToGitBashPathAdhocCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// クリップボードにコピー
	Clipboard::Copy(in->mFullPath);
	return TRUE;
}

HICON LocalToGitBashPathAdhocCommand::GetIcon()
{
	if (Path::FileExists(in->mFullPath) == FALSE) {
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

int LocalToGitBashPathAdhocCommand::Match(Pattern* pattern)
{
	CString wholeWord = pattern->GetWholeString();

	if (IsLocalPath(wholeWord) == false) {
		return Pattern::Mismatch;
	}

	static tregex patReplace(_T("^ *([a-zA-Z]):\\\\(.*)$"));
	auto driveLetter = std::regex_replace(tstring(wholeWord), patReplace, _T("$1"));
	auto path = std::regex_replace(tstring(wholeWord), patReplace, _T("$2"));

	in->mFullPath.Format(_T("/%s/%s"), driveLetter.c_str(), path.c_str());
	in->mFullPath.Replace(_T('\\'), _T('/'));

	return Pattern::WholeMatch;
}

launcherapp::core::Command*
LocalToGitBashPathAdhocCommand::Clone()
{
	auto clonedObj = make_refptr<LocalToGitBashPathAdhocCommand>();

	clonedObj->mDescription = this->mDescription;
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}

bool LocalToGitBashPathAdhocCommand::IsLocalPath(const CString& path)
{
	static tregex pat(_T("^ *[a-zA-Z]:\\\\.*$"));
	return std::regex_match(tstring(path), pat);
}


} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp


