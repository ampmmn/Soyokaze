#include "pch.h"
#include "DirectoryIndexAdhocCommand.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommandParam.h"
#include "commands/common/SubProcess.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/common/Clipboard.h"
#include "commands/core/CommandRepository.h"
#include "SharedHwnd.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

using CommandRepository = launcherapp::core::CommandRepository;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;


namespace launcherapp {
namespace commands {
namespace url_directoryindex {

struct DirectoryIndexAdhocCommand::PImpl
{
	bool EnterURL();
	bool OpenURL(const CString& url);
	bool OpenURL();

	URLDirectoryIndexCommand* mBaseCmd = nullptr;
	QueryResult mResult;
};


bool DirectoryIndexAdhocCommand::PImpl::EnterURL()
{
	auto& linkPath = mResult.mLinkPath;
	if (linkPath != _T("..") && linkPath.Right(1) != _T("/")) {
		// 次のパスがディレクトリでない場合はブラウザで開く
		return OpenURL();
	}

	// DirectoryIndexCommandに次のパスを設定する
	mBaseCmd->SetSubPath(mResult.mLinkPath);

	// ウインドウを強制的に前面に出す
	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 2, 1, 0);

	// 入力欄に"コマンド名 "のテキストを入力した状態にする
	auto cmdline = mBaseCmd->GetName();
	cmdline += _T(" ");
	SendMessage(sharedWnd.GetHwnd(), WM_APP+11, 0, (LPARAM)(LPCTSTR)cmdline);

	// 次のパスをロードする
	bool isHTML = false;
	mBaseCmd->LoadCanidates(isHTML);

	// 次のパスがHTMLファイルでなかった場合はブラウザで表示する
	if (isHTML == false) {
		return OpenURL();
	}

	// 候補の抽出が完了したことを通知
	PostMessage(sharedWnd.GetHwnd(), WM_APP+15, 0, 0);

	return true;
}

bool DirectoryIndexAdhocCommand::PImpl::OpenURL()
{
	CommandParam param;
	mBaseCmd->GetParam(param);
	CString url = param.CombineURL(mBaseCmd->GetSubPath(), mResult.mLinkPath);
	return OpenURL(url);
}

bool DirectoryIndexAdhocCommand::PImpl::OpenURL(const CString& url)
{
	RefPtr<CommandParameterBuilder> param(CommandParameterBuilder::Create(), false);

	SubProcess exec(param);
	SubProcess::ProcessPtr process;
	exec.Run(url, param->GetParameterString(), process);

	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DirectoryIndexAdhocCommand::DirectoryIndexAdhocCommand(
	URLDirectoryIndexCommand* baseCmd,
 	const QueryResult& result
) : 
	AdhocCommandBase(_T(""), _T("")),
	in(std::make_unique<PImpl>())
{
	in->mBaseCmd = baseCmd;
	baseCmd->AddRef();

	in->mResult = result;
}

DirectoryIndexAdhocCommand::~DirectoryIndexAdhocCommand()
{
	if (in->mBaseCmd) {
		in->mBaseCmd->Release();
	}
}

CString DirectoryIndexAdhocCommand::GetName()
{
	CommandParam param;
	in->mBaseCmd->GetParam(param);
	return param.mName + _T(" ") + PathFindFileName(in->mResult.mLinkText);
}

CString DirectoryIndexAdhocCommand::GetDescription()
{
	CommandParam param;
	in->mBaseCmd->GetParam(param);

	CString url = param.CombineURL(in->mBaseCmd->GetSubPath(), in->mResult.mLinkPath);

	CString str;
	str.Format(_T("%s"), (LPCTSTR)url);
	return str;

}

CString DirectoryIndexAdhocCommand::GetGuideString()
{
	return _T("Enter:開く Shift-Enter:ブラウザで開く Ctrl-Enter:URLをコピー");
}

CString DirectoryIndexAdhocCommand::GetTypeDisplayName()
{
	ASSERT(in->mBaseCmd);
	return in->mBaseCmd->GetTypeDisplayName();
}


BOOL DirectoryIndexAdhocCommand::Execute(Parameter* param)
{
	uint32_t state = GetModifierKeyState(param, MASK_ALL);

	bool isCtrlKeyPressed = state == MASK_CTRL;
	bool isShiftKeyPressed = state == MASK_SHIFT;

	if (isCtrlKeyPressed) {
		// URLをコピー
		CommandParam param_;
		in->mBaseCmd->GetParam(param_);
		Clipboard::Copy(param_.CombineURL(in->mBaseCmd->GetSubPath(), in->mResult.mLinkPath));
	}
	else if (isShiftKeyPressed) {
		// URLをブラウザで開く
		in->OpenURL();
	}
	else {
		// ランチャーでリンク先に遷移する
		in->EnterURL();
	}
	return TRUE;
}

HICON DirectoryIndexAdhocCommand::GetIcon()
{
	// リンクテキストが".."または"/"を含む場合はフォルダアイコンを表示
	auto& text = in->mResult.mLinkText;
	if (text.Find(_T("..")) == 0 || 
			(text.IsEmpty() == FALSE && text[text.GetLength()-1] == _T('/'))) {
		return IconLoader::Get()->LoadFolderIcon();
	}

	CString ext = PathFindExtension(in->mResult.mLinkPath);
	if (ext.IsEmpty() == FALSE) {
		HICON icon = IconLoader::Get()->LoadExtensionIcon(ext);
		if (icon) {
			return icon;
		}
	}

	return IconLoader::Get()->LoadUnknownIcon();
}

launcherapp::core::Command*
DirectoryIndexAdhocCommand::Clone()
{
	return new DirectoryIndexAdhocCommand(in->mBaseCmd, in->mResult);
}



} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp


