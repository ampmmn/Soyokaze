#include "pch.h"
#include "framework.h"
#include "commands/pathconvert/FileProtocolConvertAdhocCommand.h"
#include "commands/common/Clipboard.h"
#include "commands/common/Message.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using Clipboard = soyokaze::commands::common::Clipboard;
using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;

namespace soyokaze {
namespace commands {
namespace pathconvert {

struct FileProtocolConvertAdhocCommand::PImpl
{
	CString mFullPath;
};


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
	return _T("Enter:パスをコピー Shift-Enter:開く Ctrl-Enter:フォルダを開く");
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

int FileProtocolConvertAdhocCommand::Match(Pattern* pattern)
{
	CString wholeWord = pattern->GetWholeString();

	// file://ではじまるものか判断する
	static tregex patProtocol(_T("^ *file://.*"));
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

	return Pattern::WholeMatch;
}

soyokaze::core::Command*
FileProtocolConvertAdhocCommand::Clone()
{
	auto clonedObj = std::make_unique<FileProtocolConvertAdhocCommand>();
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}

} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace soyokaze


