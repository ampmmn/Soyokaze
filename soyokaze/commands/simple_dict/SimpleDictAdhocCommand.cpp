#include "pch.h"
#include "framework.h"
#include "SimpleDictAdhocCommand.h"
#include "commands/simple_dict/SimpleDictParam.h"
#include "commands/common/SubProcess.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/core/CommandRepository.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

using CommandRepository = launcherapp::core::CommandRepository;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;
using SubProcess = launcherapp::commands::common::SubProcess;


namespace launcherapp {
namespace commands {
namespace simple_dict {

struct SimpleDictAdhocCommand::PImpl
{
	const SimpleDictParam* mParam = nullptr;

	Record mRecord;
};


SimpleDictAdhocCommand::SimpleDictAdhocCommand(
	const SimpleDictParam& param,
	const Record& record
) : 
	AdhocCommandBase(_T(""), _T("")),
	in(std::make_unique<PImpl>())
{
	in->mParam = &param;
	in->mRecord = record;

	// 名前の生成
	CString tmp;
	if (param.mNameFormat.IsEmpty()) {
		tmp = _T("$key : $value");
	}
	else {
		tmp = param.mNameFormat;
	}
	tmp.Replace(_T("$key"), record.mKey);
	tmp.Replace(_T("$value2"), record.mValue2);
	tmp.Replace(_T("$value"), record.mValue);
	this->mName = param.mName + _T(" ") + tmp;

	// 説明の生成
	if (param.mDescriptionFormat.IsEmpty()) {
		tmp = _T("$key → $value");
	}
	else {
		tmp = param.mDescriptionFormat;
	}
	tmp.Replace(_T("$key"), record.mKey);
	tmp.Replace(_T("$value2"), record.mValue2);
	tmp.Replace(_T("$value"), record.mValue);
	this->mDescription = tmp;
}

SimpleDictAdhocCommand::~SimpleDictAdhocCommand()
{
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

BOOL SimpleDictAdhocCommand::Execute(Parameter* param)
{
	// Shift-Enterでキーをコピー、Ctrl-Enterで値をコピー
	// Enterキーのみ押下の場合は設定したアクションを実行
	uint32_t state = GetModifierKeyState(param, MASK_CTRL | MASK_SHIFT);
	bool isCtrlKeyPressed = (state & MASK_CTRL) != 0;
	bool isShiftKeyPressed = (state & MASK_SHIFT) != 0;
	if (isCtrlKeyPressed && isShiftKeyPressed == false) {
		// 値をコピー
		auto value = in->mRecord.mValue;
		if (in->mParam->mIsExpandMacro) {
			ExpandMacros(value);
		}
		Clipboard::Copy(value);
		return true;
	}
	if (isCtrlKeyPressed == false && isShiftKeyPressed) {
		// キーをコピー
		auto key = in->mRecord.mKey;
		if (in->mParam->mIsExpandMacro) {
			ExpandMacros(key);
		}
		Clipboard::Copy(key);
		return true;
	}

	CString argSub = in->mParam->mAfterCommandParam;
	argSub.Replace(_T("$key"), in->mRecord.mKey);
	argSub.Replace(_T("$value2"), in->mRecord.mValue2);
	argSub.Replace(_T("$value"), in->mRecord.mValue);
	if (in->mParam->mIsExpandMacro) {
		ExpandMacros(argSub);
	}

	int actionType = in->mParam->mActionType;
	if (actionType == 0) {
		// 他のコマンドを実行
		auto cmdRepo = CommandRepository::GetInstance();
		RefPtr<launcherapp::core::Command> command(cmdRepo->QueryAsWholeMatch(in->mParam->mAfterCommandName, false));
		if (command) {
			RefPtr<CommandParameterBuilder> paramSub(CommandParameterBuilder::Create(), false);

			paramSub->AddArgument(argSub);
			command->Execute(paramSub);
		}
	}
	else if (actionType == 1) {

		// 他のファイルを実行/URLを開く
		SubProcess exec(CommandParameterBuilder::EmptyParam());

		CString path = in->mParam->mAfterFilePath;
		path.Replace(_T("$key"), in->mRecord.mKey);
		path.Replace(_T("$value2"), in->mRecord.mValue2);
		path.Replace(_T("$value"), in->mRecord.mValue);
		if (in->mParam->mIsExpandMacro) {
			ExpandMacros(path);
		}

		SubProcess::ProcessPtr process;
		exec.Run(path, argSub, process);
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
	return new SimpleDictAdhocCommand(*in->mParam, in->mRecord);
}

} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

