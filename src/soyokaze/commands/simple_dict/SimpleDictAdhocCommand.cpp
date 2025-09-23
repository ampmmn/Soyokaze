#include "pch.h"
#include "framework.h"
#include "SimpleDictAdhocCommand.h"
#include "commands/simple_dict/SimpleDictParam.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/core/CommandRepository.h"
#include "actions/core/ActionParameter.h"
#include "actions/builtin/ExecuteAction.h"
#include "actions/builtin/CallbackAction.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

using CommandRepository = launcherapp::core::CommandRepository;
using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;
using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;
using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;

namespace launcherapp {
namespace commands {
namespace simple_dict {

struct SimpleDictAdhocCommand::PImpl
{
	const SimpleDictParam* mParam{nullptr};

	Record mRecord;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(SimpleDictAdhocCommand)

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
	if (in->mParam->mIsExpandMacro) {
		ExpandMacros(this->mName);
	}

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
	if (in->mParam->mIsExpandMacro) {
		ExpandMacros(this->mDescription);
	}
}

SimpleDictAdhocCommand::~SimpleDictAdhocCommand()
{
}

CString SimpleDictAdhocCommand::GetGuideString()
{
	CString guideStr(_T("⏎:実行"));
	guideStr += _T(" S-⏎:キーをコピー");
	guideStr += _T(" C-⏎:値をコピー");

	return guideStr;
}

CString SimpleDictAdhocCommand::GetTypeDisplayName()
{
	return _T("簡易辞書");
}


#pragma warning( push )
#pragma warning( disable : 26813 )

bool SimpleDictAdhocCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0) {
		// Enterキーのみ押下の場合は設定したアクションを実行
		return CreateAction(action);
	}
	else if (modifierFlags == Command::MODIFIER_SHIFT) {
		// Shift-Enterでキーをコピー
		auto key = in->mRecord.mKey;
		if (in->mParam->mIsExpandMacro) {
			ExpandMacros(key);
		}
		auto a = new CopyTextAction(key);
		a->SetDisplayName(_T("キーをコピー"));
		*action = a;
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_CTRL) {
	// Ctrl-Enterで値をコピー
		auto value = in->mRecord.mValue;
		if (in->mParam->mIsExpandMacro) {
			ExpandMacros(value);
		}
		auto a = new CopyTextAction(value);
		a->SetDisplayName(_T("値をコピー"));
		*action = a;
		return true;
	}
	return false;
}

#pragma warning( pop )

bool SimpleDictAdhocCommand::CreateAction(Action** action)
{
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
		*action = new CallbackAction(_T("実行"), [&, argSub](Parameter*, String* errMsg) -> bool {

			auto cmdRepo = CommandRepository::GetInstance();
			RefPtr<launcherapp::core::Command> command(cmdRepo->QueryAsWholeMatch(in->mParam->mAfterCommandName, false));
			if (command.get() == nullptr) {
				if (errMsg) {
					*errMsg = "コマンドが見つかりません";
				}
				return false;
			}

			RefPtr<ParameterBuilder> paramSub(ParameterBuilder::Create(), false);
			paramSub->AddArgument(argSub);

			RefPtr<Action> a;
			if (command->GetAction(0, &a) == false) {
				spdlog::error("Failed to get action.");
				return false;
			}
			return a->Perform(paramSub, errMsg);
		});
		return true;
	}
	else if (actionType == 1) {
		
		// 他のファイルを実行/URLを開く
		CString path = in->mParam->mAfterFilePath;
		path.Replace(_T("$key"), in->mRecord.mKey);
		path.Replace(_T("$value2"), in->mRecord.mValue2);
		path.Replace(_T("$value"), in->mRecord.mValue);
		if (in->mParam->mIsExpandMacro) {
			ExpandMacros(path);
		}

		*action = new ExecuteAction(path, argSub, in->mParam->mAfterDir, in->mParam->GetAfterShowType());
		return true;

	}
	else {
		// クリップボードにコピー
		*action = new CopyTextAction(argSub);
		return true;
	}
	return false;
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

CString SimpleDictAdhocCommand::GetSourceName()
{
	return in->mParam->mName;
}

bool SimpleDictAdhocCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_EXTRACANDIDATE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidate*)this;
		return true;
	}
	return false;
}


} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

