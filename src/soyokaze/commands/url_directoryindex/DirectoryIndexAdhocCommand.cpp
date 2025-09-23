#include "pch.h"
#include "DirectoryIndexAdhocCommand.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommandParam.h"
#include "commands/common/SubProcess.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/common/Clipboard.h"
#include "commands/core/CommandRepository.h"
#include "actions/core/ActionParameter.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "actions/web/OpenURLAction.h"
#include "actions/builtin/CallbackAction.h"
#include "mainwindow/controller/MainWindowController.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

using CommandRepository = launcherapp::core::CommandRepository;
using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;
using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;
using OpenURLAction = launcherapp::actions::web::OpenURLAction;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

struct DirectoryIndexAdhocCommand::PImpl
{
	CString GetCurrentURL();
	CString GetParentURL();

	bool EnterURL();
	bool CopyURL();
	bool OpenURL(const CString& url);
	bool OpenURL();
	bool OpenParentURL();

	URLDirectoryIndexCommand* mBaseCmd{nullptr};
	QueryResult mResult;
};

CString DirectoryIndexAdhocCommand::PImpl::GetCurrentURL()
{
	CommandParam param;
	mBaseCmd->GetParam(param);
	return param.CombineURL(mBaseCmd->GetSubPath(), mResult.mLinkPath);
}

CString DirectoryIndexAdhocCommand::PImpl::GetParentURL()
{
	CommandParam param;
	mBaseCmd->GetParam(param);
	CString url = param.CombineURL(mBaseCmd->GetSubPath(), mResult.mLinkPath);

  // 末尾が '/' の場合は除去
	if (!url.IsEmpty() && url.Right(1) == _T("/")) {
		url = url.Left(url.GetLength() - 1);
	}

	// 最後の '/' の位置を検索して除去
	int pos = url.ReverseFind(_T('/'));
	if (pos == url.GetLength()-1) {
		url = url.Left(url.GetLength()-1);
		pos = url.ReverseFind(_T('/'));
	}
	if (pos != -1) {
		url = url.Left(pos);
	}
	return url;
}


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
	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	bool isShowToggle = false;
	mainWnd->ActivateWindow(isShowToggle);

	// 入力欄に"コマンド名 "のテキストを入力した状態にする
	auto cmdline = mBaseCmd->GetName();
	cmdline += _T(" ");
	mainWnd->SetText(cmdline);

	// 次のパスをロードする
	bool isHTML = false;
	mBaseCmd->LoadCanidates(isHTML);

	// 次のパスがHTMLファイルでなかった場合はブラウザで表示する
	if (isHTML == false) {
		return OpenURL();
	}

	// 候補の抽出が完了したことを通知
	mainWnd->UpdateCandidateRequest();

	return true;
}

bool DirectoryIndexAdhocCommand::PImpl::CopyURL()
{
		CommandParam param;
		mBaseCmd->GetParam(param);
		Clipboard::Copy(param.CombineURL(mBaseCmd->GetSubPath(), mResult.mLinkPath));
		return true;
}

bool DirectoryIndexAdhocCommand::PImpl::OpenURL()
{
	CommandParam param;
	mBaseCmd->GetParam(param);
	CString url = param.CombineURL(mBaseCmd->GetSubPath(), mResult.mLinkPath);
	return OpenURL(url);
}

bool DirectoryIndexAdhocCommand::PImpl::OpenParentURL()
{
	CommandParam param;
	mBaseCmd->GetParam(param);
	CString url = param.CombineURL(mBaseCmd->GetSubPath(), mResult.mLinkPath);
	int pos = url.ReverseFind(_T('/'));
	if (pos != -1) {
		url = url.Left(pos);
	}
	return OpenURL(url);
}

bool DirectoryIndexAdhocCommand::PImpl::OpenURL(const CString& url)
{
	SubProcess exec(ParameterBuilder::EmptyParam());
	SubProcess::ProcessPtr process;
	exec.Run(url, process);

	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(DirectoryIndexAdhocCommand)

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

CString DirectoryIndexAdhocCommand::GetTypeDisplayName()
{
	ASSERT(in->mBaseCmd);
	return in->mBaseCmd->GetTypeDisplayName();
}


bool DirectoryIndexAdhocCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == Command::MODIFIER_CTRL) {
		// URLをコピー
		*action = new CopyTextAction(in->GetCurrentURL());
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_SHIFT) {
		// URLをブラウザで開く
		auto a = new OpenURLAction(in->GetCurrentURL());;
		a->SetDisplayName(_T("リンク先をブラウザで開く"));
		*action = a;
		return true;
	}
	else if (modifierFlags == (Command::MODIFIER_SHIFT | Command::MODIFIER_CTRL)) {
		// ディレクトリをブラウザで開く
		auto a = new OpenURLAction(in->GetParentURL());
		a->SetDisplayName(_T("現在の階層をブラウザで開く"));
		*action = a;
		return true;
	}
	else if (modifierFlags == 0) {
		// ランチャーでリンク先に遷移する
		*action = new CallbackAction(_T("開く"), [&](Parameter*, String*) -> bool {
			in->EnterURL();
			return true;
		});
		return true;
	}
	return false;
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

// メニューの項目数を取得する
int DirectoryIndexAdhocCommand::GetMenuItemCount()
{
	return 4;
}

// メニューの表示名を取得する
bool DirectoryIndexAdhocCommand::GetMenuItem(int index, Action** action)
{
	if (index < 0 || GetMenuItemCount() < index) {
		return false;
	}

	if (index == 0) {
		*action = new CallbackAction(_T("開く"), [&](Parameter*, String*) -> bool {
			return in->EnterURL();
		});
		return true;
	}
	else if (index == 1) {
		*action = new CopyTextAction(in->GetCurrentURL());
		return true;
	}
	else if (index == 2) {
		auto a = new OpenURLAction(in->GetCurrentURL());
		a->SetDisplayName(_T("リンク先をブラウザで開く"));
		*action = a;
		return true;
	}
	else if (index == 3) {
		auto a = new OpenURLAction(in->GetParentURL());
		a->SetDisplayName(_T("現在の階層をブラウザで開く"));
		*action = a;
		return true;
	}
	return false;
}

CString DirectoryIndexAdhocCommand::GetSourceName()
{
	return in->mBaseCmd->GetName();
}


bool DirectoryIndexAdhocCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_CONTEXTMENUSOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ContextMenuSource*)this;
		return true;
	}
	else if (ifid == IFID_EXTRACANDIDATE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidate*)this;
		return true;
	}
	return false;
}


} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp


