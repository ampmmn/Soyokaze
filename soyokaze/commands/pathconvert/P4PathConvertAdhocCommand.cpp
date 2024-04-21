#include "pch.h"
#include "framework.h"
#include "P4PathConvertAdhocCommand.h"
#include "commands/pathconvert/P4AppSettings.h"
#include "commands/common/Clipboard.h"
#include "commands/common/Message.h"
#include "commands/shellexecute/ShellExecCommand.h"
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

struct P4PathConvertAdhocCommand::PImpl
{
	ITEM mItem;
	CString mFullPath;
};

P4PathConvertAdhocCommand::P4PathConvertAdhocCommand() : in(std::make_unique<PImpl>())
{
}

P4PathConvertAdhocCommand::P4PathConvertAdhocCommand(const ITEM& item) : in(std::make_unique<PImpl>())
{
	in->mItem = item;
}

P4PathConvertAdhocCommand::~P4PathConvertAdhocCommand()
{
}


CString P4PathConvertAdhocCommand::GetName()
{
	return in->mFullPath;
}

CString P4PathConvertAdhocCommand::GetGuideString()
{
	return _T("Enter:パスをコピー Shift-Enter:開く Ctrl-Enter:フォルダを開く");
}

CString P4PathConvertAdhocCommand::GetTypeDisplayName()
{
	return _T("パス変換(p4)");
}

BOOL P4PathConvertAdhocCommand::Execute(const Parameter& param)
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

HICON P4PathConvertAdhocCommand::GetIcon()
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

int P4PathConvertAdhocCommand::Match(Pattern* pattern)
{
	// ToDo: 変換
	return Pattern::Mismatch;
}

launcherapp::core::Command*
P4PathConvertAdhocCommand::Clone()
{
	auto clonedObj = std::make_unique<P4PathConvertAdhocCommand>();

	clonedObj->in->mFullPath = in->mFullPath;
	clonedObj->in->mItem = in->mItem;

	return clonedObj.release();
}


} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp


