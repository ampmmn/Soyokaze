#include "pch.h"
#include "framework.h"
#include "commands/pathfind/CydriveToLocalPathAdhocCommand.h"
#include "commands/common/SubProcess.h"
#include "commands/common/Clipboard.h"
#include "commands/common/Message.h"
#include "AppPreference.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


using SubProcess = soyokaze::commands::common::SubProcess;
using Clipboard = soyokaze::commands::common::Clipboard;


namespace soyokaze {
namespace commands {
namespace pathfind {

struct CydriveToLocalPathAdhocCommand::PImpl
{
	bool IsEnable();

	CString mFullPath;
	bool mIsExe;
};


bool CydriveToLocalPathAdhocCommand::PImpl::IsEnable()
{
	auto pref = AppPreference::Get();
	return pref->IsEnableCygwinPath();
}

CydriveToLocalPathAdhocCommand::CydriveToLocalPathAdhocCommand() : in(std::make_unique<PImpl>())
{
	in->mIsExe = false;
}

CydriveToLocalPathAdhocCommand::~CydriveToLocalPathAdhocCommand()
{
}


CString CydriveToLocalPathAdhocCommand::GetName()
{
	return in->mFullPath;
}

CString CydriveToLocalPathAdhocCommand::GetGuideString()
{
	return _T("Enter:パスをコピー Ctrl-Enter:フォルダを開く Shift-Enter:開く");
}

CString CydriveToLocalPathAdhocCommand::GetTypeDisplayName()
{
	return _T("Cygwin To Local Path");
}

bool CydriveToLocalPathAdhocCommand::ShouldCopy(const Parameter& param)
{
	// 何も修飾キーがおされてないならコピー操作をする
	return param.GetNamedParamBool(_T("CtrlKeyPressed")) == false &&
	       param.GetNamedParamBool(_T("ShiftKeyPressed")) == false &&
	       param.GetNamedParamBool(_T("AltKeyPressed")) == false &&
	       param.GetNamedParamBool(_T("WinKeyPressed")) == false;
}

BOOL CydriveToLocalPathAdhocCommand::Execute(const Parameter& param)
{
	if (ShouldCopy(param)) {
		// クリップボードにコピー
		Clipboard::Copy(in->mFullPath);
		return TRUE;
	}
	else {
		// フォルダを開く or 開く
		if (PathFileExists(in->mFullPath) == FALSE) {
			soyokaze::commands::common::PopupMessage(_T("パスは存在しません"));
			return TRUE;
		}

		SubProcess exec(param);
		SubProcess::ProcessPtr process;
		if (exec.Run(in->mFullPath, param.GetParameterString(), process) == FALSE) {
			this->mErrMsg = (LPCTSTR)process->GetErrorMessage();
			return FALSE;
		}
	}
	return TRUE;
}

HICON CydriveToLocalPathAdhocCommand::GetIcon()
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

int CydriveToLocalPathAdhocCommand::Match(Pattern* pattern)
{
	if (in->IsEnable() == false) {
		return Pattern::Mismatch;
	}

	CString wholeWord = pattern->GetWholeString();

	// /cygdrive/...に合致するか
	if (IsCygdrivePath(wholeWord)) {

		tregex patReplace(_T("^ */cygdrive/([a-zA-Z])/(.*)$"));
		auto driveLetter = std::regex_replace(tstring(wholeWord), patReplace, _T("$1"));
		auto path = std::regex_replace(tstring(wholeWord), patReplace, _T("$2"));

		in->mFullPath.Format(_T("%s:\\%s"), driveLetter.c_str(), path.c_str());
		in->mFullPath.Replace(_T('/'), _T('\\'));

		return Pattern::WholeMatch;
	}
	return Pattern::Mismatch;
}

soyokaze::core::Command*
CydriveToLocalPathAdhocCommand::Clone()
{
	auto clonedObj = std::make_unique<CydriveToLocalPathAdhocCommand>();

	clonedObj->mDescription = this->mDescription;
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}

bool CydriveToLocalPathAdhocCommand::IsCygdrivePath(const CString& path)
{
	static tregex pat(_T("^ */cygdrive/[a-zA-Z]/.*$"));
	return std::regex_match(tstring(path), pat);
}


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace soyokaze


