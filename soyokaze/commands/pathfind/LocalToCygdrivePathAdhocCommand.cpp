#include "pch.h"
#include "framework.h"
#include "commands/pathfind/LocalToCygdrivePathAdhocCommand.h"
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

struct LocalToCygdrivePathAdhocCommand::PImpl
{
	bool IsEnable();

	CString mFullPath;
};


bool LocalToCygdrivePathAdhocCommand::PImpl::IsEnable()
{
	auto pref = AppPreference::Get();
	return pref->IsEnableCygwinPath();
}

LocalToCygdrivePathAdhocCommand::LocalToCygdrivePathAdhocCommand() : in(std::make_unique<PImpl>())
{
}

LocalToCygdrivePathAdhocCommand::~LocalToCygdrivePathAdhocCommand()
{
}


CString LocalToCygdrivePathAdhocCommand::GetName()
{
	return in->mFullPath;
}

CString LocalToCygdrivePathAdhocCommand::GetGuideString()
{
	return _T("Enter:パスをコピー");
}

CString LocalToCygdrivePathAdhocCommand::GetTypeDisplayName()
{
	return _T("Local To Cygwin Path");
}

BOOL LocalToCygdrivePathAdhocCommand::Execute(const Parameter& param)
{
	// クリップボードにコピー
	Clipboard::Copy(in->mFullPath);
	return TRUE;
}

HICON LocalToCygdrivePathAdhocCommand::GetIcon()
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

int LocalToCygdrivePathAdhocCommand::Match(Pattern* pattern)
{
	if (in->IsEnable() == false) {
		return Pattern::Mismatch;
	}

	CString wholeWord = pattern->GetWholeString();

	// /cygdrive/...に合致するか
	if (IsLocalPath(wholeWord) == false) {
		return Pattern::Mismatch;
	}

	static tregex patReplace(_T("^ *([a-zA-Z]):\\\\(.*)$"));
	auto driveLetter = std::regex_replace(tstring(wholeWord), patReplace, _T("$1"));
	auto path = std::regex_replace(tstring(wholeWord), patReplace, _T("$2"));

	in->mFullPath.Format(_T("/cygdrive/%s/%s"), driveLetter.c_str(), path.c_str());
	in->mFullPath.Replace(_T('\\'), _T('/'));

	return Pattern::WholeMatch;
}

soyokaze::core::Command*
LocalToCygdrivePathAdhocCommand::Clone()
{
	auto clonedObj = std::make_unique<LocalToCygdrivePathAdhocCommand>();

	clonedObj->mDescription = this->mDescription;
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}

bool LocalToCygdrivePathAdhocCommand::IsLocalPath(const CString& path)
{
	static tregex pat(_T("^ *[a-zA-Z]:\\\\.*$"));
	return std::regex_match(tstring(path), pat);
}


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace soyokaze


