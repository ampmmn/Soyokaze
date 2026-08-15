#include "pch.h"
#include "framework.h"
#include "commands/pathconvert/LocalToGitBashPathAdhocCommand.h"
#include "commands/pathconvert/Icon.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "icon/IconLoader.h"
#include "utility/Path.h"
#include "utility/Regex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;

namespace launcherapp {
namespace commands {
namespace pathconvert {

struct LocalToGitBashPathAdhocCommand::PImpl
{
	CString mFullPath;
	Icon mIcon;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(LocalToGitBashPathAdhocCommand)

LocalToGitBashPathAdhocCommand::LocalToGitBashPathAdhocCommand() : in(std::make_unique<PImpl>())
{
}

LocalToGitBashPathAdhocCommand::~LocalToGitBashPathAdhocCommand()
{
}


CString LocalToGitBashPathAdhocCommand::GetName()
{
	return in->mFullPath;
}

CString LocalToGitBashPathAdhocCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool LocalToGitBashPathAdhocCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() == 0) {
		// クリップボードにコピー
		*action = new CopyTextAction(in->mFullPath);
		return true;
	}
	return false;
}

HICON LocalToGitBashPathAdhocCommand::GetIcon()
{
	if (Path::FileExists(in->mFullPath) == FALSE) {
		// dummy
		return IconLoader::Get()->LoadUnknownIcon();
	}
	return in->mIcon.Load(in->mFullPath);
}

int LocalToGitBashPathAdhocCommand::Match(Pattern* pattern)
{
	CString wholeWord = pattern->GetWholeString();

	if (IsLocalPath(wholeWord) == false) {
		return Pattern::Mismatch;
	}

	static const launcherapp::utility::Regex patReplace(_T("^ *([a-zA-Z]):\\\\(.*)$"));
	std::vector<CString> captures;
	if (patReplace.FullMatch(wholeWord, captures) == false || captures.size() < 2) {
		return Pattern::Mismatch;
	}
	auto driveLetter = tstring((LPCTSTR)captures[0]);
	auto path = tstring((LPCTSTR)captures[1]);

	in->mFullPath.Format(_T("/%s/%s"), driveLetter.c_str(), path.c_str());
	in->mFullPath.Replace(_T('\\'), _T('/'));

	return Pattern::FrontMatch;
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
	static const launcherapp::utility::Regex pat(_T("^ *[a-zA-Z]:\\\\.*$"));
	return pat.FullMatch(path);
}

CString LocalToGitBashPathAdhocCommand::TypeDisplayName()
{
	return _T("Local To git-bash Path");
}


} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp


