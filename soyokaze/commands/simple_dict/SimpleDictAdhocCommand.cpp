#include "pch.h"
#include "framework.h"
#include "SimpleDictAdhocCommand.h"
#include "commands/simple_dict/SimpleDictParam.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/Clipboard.h"
#include "commands/core/CommandRepository.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using ShellExecCommand = launcherapp::commands::shellexecute::ShellExecCommand;

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace simple_dict {

struct SimpleDictAdhocCommand::PImpl
{
	SimpleDictParam mParam;

	CString mKey;   // キー
	CString mValue; // 値
};


SimpleDictAdhocCommand::SimpleDictAdhocCommand(
	const CString& key,
	const CString& value
) : 
	AdhocCommandBase(_T(""), _T("")),
	in(std::make_unique<PImpl>())
{
	in->mKey = key;
	in->mValue = value;
}

SimpleDictAdhocCommand::~SimpleDictAdhocCommand()
{
}

void SimpleDictAdhocCommand::SetParam(const SimpleDictParam& param)
{
	in->mParam = param;
}

CString SimpleDictAdhocCommand::GetName()
{
	return in->mParam.mName + _T(" ") + in->mKey + _T(" : ") + in->mValue;
}

CString SimpleDictAdhocCommand::GetDescription()
{
	CString str;
	str.Format(_T("%s → %s"), in->mKey, in->mValue);
	return str;

}

CString SimpleDictAdhocCommand::GetGuideString()
{
	CString guideStr(_T("Enter:実行"));
	guideStr += _T(" Shift-Enter:キーをコピー");
	guideStr += _T(" Ctrl-Enter:値をコピー");

	return guideStr;
}

CString SimpleDictAdhocCommand::GetTypeDisplayName()
{
	return _T("簡易辞書");
}

BOOL SimpleDictAdhocCommand::Execute(const Parameter& param)
{
	// Shift-Enterでキーをコピー、Ctrl-Enterで値をコピー
	// Enterキーのみ押下の場合は設定したアクションを実行
	bool isCtrlKeyPressed = param.GetNamedParamBool(_T("CtrlKeyPressed"));
	bool isShiftKeyPressed = param.GetNamedParamBool(_T("ShiftKeyPressed"));
	if (isCtrlKeyPressed && isShiftKeyPressed == false) {
		// 値をコピー
		Clipboard::Copy(in->mValue);
		return true;
	}
	if (isCtrlKeyPressed == false && isShiftKeyPressed) {
		// キーをコピー
		Clipboard::Copy(in->mKey);
		return true;
	}

	CString argSub = in->mParam.mAfterCommandParam;
	argSub.Replace(_T("$key"), in->mKey);
	argSub.Replace(_T("$value"), in->mValue);
	ExpandMacros(argSub);

	int actionType = in->mParam.mActionType;
	if (actionType == 0) {
		// 他のコマンドを実行
		auto cmdRepo = CommandRepository::GetInstance();
		auto command = cmdRepo->QueryAsWholeMatch(in->mParam.mAfterCommandName, false);
		if (command) {
			Parameter paramSub;
			paramSub.AddArgument(argSub);
			command->Execute(paramSub);
			command->Release();
		}
	}
	else if (actionType == 1) {
		// 他のファイルを実行/URLを開く
		ShellExecCommand::ATTRIBUTE attr;

		attr.mPath = in->mParam.mAfterFilePath;
		attr.mPath.Replace(_T("$key"), in->mKey);
		attr.mPath.Replace(_T("$value"), in->mValue);
		ExpandMacros(attr.mPath);

		attr.mParam = argSub;

		ShellExecCommand cmd;
		cmd.SetAttribute(attr);

		Parameter paramEmpty;
		cmd.Execute(paramEmpty);
	}
	else {
		// クリップボードにコピー
		Clipboard::Copy(argSub);
	}

	return TRUE;
}

HICON SimpleDictAdhocCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-5301);
}

launcherapp::core::Command*
SimpleDictAdhocCommand::Clone()
{
	return new SimpleDictAdhocCommand(in->mKey, in->mValue);
}

} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

