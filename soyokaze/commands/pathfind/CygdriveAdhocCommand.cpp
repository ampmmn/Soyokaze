#include "pch.h"
#include "framework.h"
#include "commands/pathfind/CydriveAdhocCommand.h"
#include "commands/common/SubProcess.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using SubProcess = soyokaze::commands::common::SubProcess;

namespace soyokaze {
namespace commands {
namespace pathfind {

struct CydriveAdhocCommand::PImpl
{
	bool IsEnable();
	CString mFullPath;
	bool mIsExe;
};


bool CydriveAdhocCommand::PImpl::IsEnable()
{
	// ToDo: 切り替えられるようにする
	return true;
}

CydriveAdhocCommand::CydriveAdhocCommand() : in(std::make_unique<PImpl>())
{
	in->mIsExe = false;
}

CydriveAdhocCommand::~CydriveAdhocCommand()
{
}


CString CydriveAdhocCommand::GetName()
{
	if (in->mFullPath.IsEmpty()) {
		return _T("");
	}
	return PathFindFileName(in->mFullPath);
}

CString CydriveAdhocCommand::GetGuideString()
{
	if (in->mIsExe) {
		return _T("Enter:開く Ctrl-Enter:フォルダを開く");
	}
	else {
		return _T("Enter:開く Ctrl-Enter:フォルダを開く");
	}
}

CString CydriveAdhocCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE_ADHOC((LPCTSTR)IDS_COMMAND_PATHEXEC);
	return in->TEXT_TYPE_ADHOC;
}

BOOL CydriveAdhocCommand::Execute(const Parameter& param)
{
	if (PathFileExists(in->mFullPath) == FALSE) {
		return FALSE;
	}

	SubProcess exec(param);
	SubProcess::ProcessPtr process;
	if (exec.Run(in->mFullPath, param.GetParameterString(), process) == FALSE) {
		//in->mErrMsg = (LPCTSTR)process->GetErrorMessage();
		return FALSE;
	}

	return TRUE;
}

HICON CydriveAdhocCommand::GetIcon()
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

int CydriveAdhocCommand::Match(Pattern* pattern)
{
	if (in->IsEnable() == false) {
		return Pattern::Mismatch;
	}

	CString wholeWord = pattern->GetWholeString();

	// /cygdrive/...に合致するか
	if (IsCygdrivePath(wholeWord)) {

		tregex patReplace(_T("^ *\/cygdrive\/([a-zA-Z]\)/(.*)$"));
		auto driveLetter = std::regex_replace(wholeWord, patReplace, _T("$1"));
		auto path = std::regex_replace(wholeWord, patReplace, _T("$2"));

		CString path;
		path.Format(_T("%s:%s"), driveLetter.c_str(), path.c_str());
		path.Replace(_T('/'), _T('\\'));

		in->mFullPath = path.c_str();

		return Pattern::WholeMatch;
	}
	return Pattern::Mismatch;
}

soyokaze::core::Command*
CydriveAdhocCommand::Clone()
{
	auto clonedObj = std::make_unique<CydriveAdhocCommand>();

	clonedObj->mDescription = this->mDescription;
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}

bool CydriveAdhocCommand::IsCygdrivePath(const CString& path)
{
	static tregex pat(_T("^ *\/cygdrive\/[a-zA-Z]\/.*$"));
	return std::regex_match(tstring(path), pat);
}


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace soyokaze


