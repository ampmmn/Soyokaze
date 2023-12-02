#include "pch.h"
#include "framework.h"
#include "commands/pathfind/LocalToGitBashPathAdhocCommand.h"
#include "commands/common/Clipboard.h"
#include "commands/common/Message.h"
#include "AppPreference.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


using Clipboard = soyokaze::commands::common::Clipboard;

namespace soyokaze {
namespace commands {
namespace pathfind {

struct LocalToGitBashPathAdhocCommand::PImpl
{
	bool IsEnable();

	CString mFullPath;
};


bool LocalToGitBashPathAdhocCommand::PImpl::IsEnable()
{
	auto pref = AppPreference::Get();
	return pref->IsEnableGitBashPath();
}

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

CString LocalToGitBashPathAdhocCommand::GetGuideString()
{
	return _T("Enter:パスをコピー");
}

CString LocalToGitBashPathAdhocCommand::GetTypeDisplayName()
{
	return _T("Local To git-bash Path");
}

BOOL LocalToGitBashPathAdhocCommand::Execute(const Parameter& param)
{
	// クリップボードにコピー
	Clipboard::Copy(in->mFullPath);
	return TRUE;
}

HICON LocalToGitBashPathAdhocCommand::GetIcon()
{
	if (PathFileExists(in->mFullPath) == FALSE) {
		// dummy
		return IconLoader::Get()->LoadUnknownIcon();
	}

	SHFILEINFO sfi = {};
	HIMAGELIST hImgList =
		(HIMAGELIST)::SHGetFileInfo(in->mFullPath, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON);
	HICON hIcon = sfi.hIcon;
	return hIcon;
}

int LocalToGitBashPathAdhocCommand::Match(Pattern* pattern)
{
	if (in->IsEnable() == false) {
		return Pattern::Mismatch;
	}

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

soyokaze::core::Command*
LocalToGitBashPathAdhocCommand::Clone()
{
	auto clonedObj = std::make_unique<LocalToGitBashPathAdhocCommand>();

	clonedObj->mDescription = this->mDescription;
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}

bool LocalToGitBashPathAdhocCommand::IsLocalPath(const CString& path)
{
	static tregex pat(_T("^ *[a-zA-Z]:\\\\.*$"));
	return std::regex_match(tstring(path), pat);
}


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace soyokaze


