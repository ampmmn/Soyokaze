#include "pch.h"
#include "framework.h"
#include "commands/pathconvert/GitBashToLocalPathAdhocCommand.h"
#include "commands/pathconvert/Icon.h"
#include "actions/builtin/ExecuteAction.h"
#include "actions/builtin/OpenPathInFilerAction.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "utility/Path.h"
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

struct GitBashToLocalPathAdhocCommand::PImpl
{
	CString mFullPath;
	Icon mIcon;
	bool mIsExe{false};
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(GitBashToLocalPathAdhocCommand)

GitBashToLocalPathAdhocCommand::GitBashToLocalPathAdhocCommand() : in(std::make_unique<PImpl>())
{
}

GitBashToLocalPathAdhocCommand::~GitBashToLocalPathAdhocCommand()
{
}


CString GitBashToLocalPathAdhocCommand::GetName()
{
	return in->mFullPath;
}

CString GitBashToLocalPathAdhocCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool GitBashToLocalPathAdhocCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0) {
		// パスをクリップボードにコピー
		*action = new CopyTextAction(in->mFullPath);
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_SHIFT) {
		// 実行
		*action = new ExecuteAction(in->mFullPath);
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_CTRL) {
		// パスを開く
		*action = new OpenPathInFilerAction(in->mFullPath);
		return true;
	}
	return false;
}

HICON GitBashToLocalPathAdhocCommand::GetIcon()
{
	if (Path::FileExists(in->mFullPath) == FALSE) {
		// dummy
		return IconLoader::Get()->LoadUnknownIcon();
	}

	return in->mIcon.Load(in->mFullPath);
}

int GitBashToLocalPathAdhocCommand::Match(Pattern* pattern)
{
	CString wholeWord = pattern->GetWholeString();

	// /driveletter/...に合致するか
	if (IsGitBashPath(wholeWord)) {

		static tregex patReplace(_T("^ */([a-zA-Z])/(.*)$"));
		auto driveLetter = std::regex_replace(tstring(wholeWord), patReplace, _T("$1"));
		auto path = std::regex_replace(tstring(wholeWord), patReplace, _T("$2"));

		in->mFullPath.Format(_T("%s:\\%s"), driveLetter.c_str(), path.c_str());
		in->mFullPath.Replace(_T('/'), _T('\\'));

		return Pattern::WholeMatch;
	}
	return Pattern::Mismatch;
}

launcherapp::core::Command*
GitBashToLocalPathAdhocCommand::Clone()
{
	auto clonedObj = make_refptr<GitBashToLocalPathAdhocCommand>();

	clonedObj->mDescription = this->mDescription;
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}

bool GitBashToLocalPathAdhocCommand::IsGitBashPath(const CString& path)
{
	static tregex pat(_T("^ */[a-zA-Z]/.*$"));
	return std::regex_match(tstring(path), pat);
}

CString GitBashToLocalPathAdhocCommand::TypeDisplayName()
{
	return _T("git-bash To Local Path");
}


} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp


