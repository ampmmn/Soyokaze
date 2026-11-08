#include "pch.h"
#include "OutputPreviewResultCommand.h"
#include "commands/common/preview/PreviewWindow.h"
#include "commands/common/preview/WebviewPreviewWindow.h"
#include "commands/output_preview/OutputPreviewRuntime.h"
#include "actions/builtin/CallbackAction.h"
#include "commands/common/Clipboard.h"
#include "core/IFIDDefine.h"
#include "icon/IconLoader.h"
#include <shlwapi.h>

namespace launcherapp {
namespace commands {
namespace output_preview {

struct OutputPreviewResultCommand::PImpl
{
	OutputPreviewRuntime::Result mResult;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(OutputPreviewResultCommand)

OutputPreviewResultCommand::OutputPreviewResultCommand(const OutputPreviewRuntime::Result& result) :
	AdhocCommandBase(result.mCommandName, _T("実行結果を表示しています")),
	in(std::make_unique<PImpl>())
{
	in->mResult = result;
}

OutputPreviewResultCommand::~OutputPreviewResultCommand()
{
}

bool OutputPreviewResultCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}
	if (ifid == IFID_PREVIEWPROVIDER) {
		AddRef();
		*cmd = static_cast<launcherapp::core::PreviewProvider*>(this);
		return true;
	}
	if (ifid == IFID_SELECTIONBEHAVIOR) {
		AddRef();
		*cmd = static_cast<launcherapp::core::SelectionBehavior*>(this);
		return true;
	}
	if (ifid == IFID_OUTPUTPREVIEWRESULTCOMMAND) {
		AddRef();
		*cmd = static_cast<OutputPreviewResultCommand*>(this);
		return true;
	}
	return false;
}

CString OutputPreviewResultCommand::GetTypeDisplayName()
{
	return _T("実行結果確認コマンド");
}

bool OutputPreviewResultCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	const auto resultName = in->mResult.mName;
	if (hotkeyAttr.GetModifiers() == MOD_SHIFT) {
		const auto text = in->mResult.mText;
		*action = new launcherapp::actions::builtin::CallbackAction(
			_T("クリップボードにコピーして閉じる"), [resultName, text](Parameter*, String* errMsg) {
				if (launcherapp::commands::common::Clipboard::Copy(text) == false) {
					if (errMsg) {
						*errMsg = _T("クリップボードにコピーできませんでした");
					}
					return false;
				}
				OutputPreviewRuntime::Get()->Close(resultName);
				launcherapp::commands::common::preview::WebviewPreviewWindow::Get()->Clear();
				return true;
			});
		return true;
	}

	if (hotkeyAttr.GetModifiers() != 0) {
		return false;
	}
	*action = new launcherapp::actions::builtin::CallbackAction(
		_T("プレビューを閉じる"), [resultName](Parameter*, String*) {
			OutputPreviewRuntime::Get()->Close(resultName);
			launcherapp::commands::common::preview::WebviewPreviewWindow::Get()->Clear();
			return true;
		});
	return true;
}

HICON OutputPreviewResultCommand::GetIcon()
{
	return in->mResult.mExecutablePath.IsEmpty() ? IconLoader::Get()->LoadPromptIcon() :
		IconLoader::Get()->LoadIconFromPath(in->mResult.mExecutablePath);
}

launcherapp::core::Command* OutputPreviewResultCommand::Clone()
{
	return new OutputPreviewResultCommand(in->mResult);
}

launcherapp::mainwindow::preview::PreviewWindow* OutputPreviewResultCommand::FetchPreviewWindow()
{
	if (in->mResult.mPreviewType == 0) {
	auto previewWindow = launcherapp::commands::common::preview::PreviewWindow::Get();
		previewWindow->SetPreviewText(in->mResult.mText, _T("実行結果"), {});
		return previewWindow;
	}

	if (in->mResult.mFilePath.IsEmpty()) {
		return nullptr;
	}
	std::vector<TCHAR> urlBuffer(32768);
	DWORD urlLength = static_cast<DWORD>(urlBuffer.size());
	if (UrlCreateFromPath(in->mResult.mFilePath, urlBuffer.data(), &urlLength, 0) != S_OK) {
		return nullptr;
	}
	auto previewWindow = launcherapp::commands::common::preview::WebviewPreviewWindow::Get();
	previewWindow->SetFileUrl(urlBuffer.data());
	return previewWindow;
}

void OutputPreviewResultCommand::OnSelect(launcherapp::core::Command*)
{
}

void OutputPreviewResultCommand::OnUnselect(launcherapp::core::Command*)
{
	OutputPreviewRuntime::Get()->Close(in->mResult.mName);
	launcherapp::commands::common::preview::PreviewWindow::Get()->SetPreviewText(_T(""), _T(""), {});
	launcherapp::commands::common::preview::WebviewPreviewWindow::Get()->Clear();
}

launcherapp::core::SelectionBehavior::CloseWindowPolicy
OutputPreviewResultCommand::GetCloseWindowPolicy(uint32_t)
{
	return launcherapp::core::SelectionBehavior::CLOSEWINDOW_ASYNC;
}

bool OutputPreviewResultCommand::CompleteKeyword(CString&, int&, int&)
{
	return false;
}

}
}
}
