#include "pch.h"
#include "OutputPreviewCommandProvider.h"
#include "OutputPreviewCommand.h"
#include "OutputPreviewResultCommand.h"
#include "OutputPreviewRuntime.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace output_preview {

REGISTER_COMMANDPROVIDER(OutputPreviewCommandProvider)

OutputPreviewCommandProvider::OutputPreviewCommandProvider()
{
}

OutputPreviewCommandProvider::~OutputPreviewCommandProvider()
{
}

CString OutputPreviewCommandProvider::GetName()
{
	return _T("OutputPreviewCommand");
}

CString OutputPreviewCommandProvider::GetDisplayName()
{
	return _T("実行結果確認コマンド");
}

CString OutputPreviewCommandProvider::GetDescription()
{
	return _T("ファイルの実行結果を表示するコマンドです。\n")
		_T("実行したプログラムの出力結果(標準出力または出力ファイル)を\n")
		_T("本アプリ内のプレビューウインドウで結果を確認できます。\n")
		_T("(なお、本コマンドではURLを直接指定したリクエスト実行をサポートしていません)");
}

bool OutputPreviewCommandProvider::NewDialog(Parameter*)
{
	auto command = std::make_unique<OutputPreviewCommand>();
	RefPtr<launcherapp::core::CommandEditor> editor;
	if (command->CreateEditor(nullptr, &editor) == false || editor->DoModal() == false ||
		command->Apply(editor.get()) == false) {
		return false;
	}
	launcherapp::core::CommandRepository::GetInstance()->RegisterCommand(command.release());
	return true;
}

uint32_t OutputPreviewCommandProvider::GetOrder() const
{
	return 130;
}

uint32_t OutputPreviewCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(_T("実行結果確認コマンド"));
	return 1;
}

void OutputPreviewCommandProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands)
{
	CString resultName;
	CString wholeString(pattern->GetWholeString());
	if (OutputPreviewRuntime::Get()->GetResultName(resultName) == false) {
		return;
	}

	OutputPreviewRuntime::Result result;
	if (OutputPreviewRuntime::Get()->Fetch(resultName, result) == false ||
		wholeString.CompareNoCase(result.mInputText) != 0) {
		return;
	}
	commands.Add(CommandQueryItem(Pattern::WholeMatch, new OutputPreviewResultCommand(result)));
}

void OutputPreviewCommandProvider::OnBeforeLoad()
{
}

bool OutputPreviewCommandProvider::LoadFrom(CommandEntryIF* entry, Command** command)
{
	std::unique_ptr<OutputPreviewCommand> outputPreview(new OutputPreviewCommand());
	if (outputPreview->Load(entry) == false) {
		return false;
	}
	*command = outputPreview.release();
	return true;
}

}
}
}
