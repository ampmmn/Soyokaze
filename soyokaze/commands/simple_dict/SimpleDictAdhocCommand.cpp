#include "pch.h"
#include "framework.h"
#include "SimpleDictAdhocCommand.h"
#include "commands/simple_dict/SimpleDictParam.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/Clipboard.h"
#include "core/CommandRepository.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace soyokaze::commands::common;
using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;

using CommandRepository = soyokaze::core::CommandRepository;

namespace soyokaze {
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
	CString guideStr;

	int actionType = in->mParam.mActionType;
	if (actionType == 0) {
		guideStr.Format(_T("Enter:%sコマンドを実行"), in->mParam.mAfterCommandName);
		return guideStr;
	}
	else if (actionType == 1) {
		return _T("Enter:プログラムを実行");
	}
	else {
		guideStr.Format(_T("Enter:クリップボードに文字列をコピー→ \"%s\""), in->mValue);
		return guideStr;
	}
}

CString SimpleDictAdhocCommand::GetTypeDisplayName()
{
	return _T("簡易辞書");
}

BOOL SimpleDictAdhocCommand::Execute(const Parameter& param)
{
	CString argSub = in->mParam.mAfterCommandParam;
	argSub.Replace(_T("$key"), in->mKey);
	argSub.Replace(_T("$value"), in->mValue);
	ExpandAfxCurrentDir(argSub);

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
		ExpandAfxCurrentDir(attr.mPath);

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

soyokaze::core::Command*
SimpleDictAdhocCommand::Clone()
{
	return new SimpleDictAdhocCommand(in->mKey, in->mValue);
}

} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace soyokaze

