#include "pch.h"
#include "SimpleDictCommand.h"
#include "commands/simple_dict/SimpleDictEditDialog.h"
#include "commands/simple_dict/ExcelWrapper.h"
#include "commands/simple_dict/SimpleDictCommandUpdateListenerIF.h"
#include "core/CommandRepository.h"
#include "core/CommandRepositoryListenerIF.h"
#include "utility/ScopeAttachThreadInput.h"
#include "AppPreference.h"
#include "CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "commands/common/Message.h"
#include <assert.h>
#include <regex>
#include <set>

namespace soyokaze {
namespace commands {
namespace simple_dict {

using CommandRepositoryListenerIF = soyokaze::core::CommandRepositoryListenerIF;

struct SimpleDictCommand::PImpl : public CommandRepositoryListenerIF
{
	void OnDeleteCommand(Command* command) override
	{
		if (command != mThisPtr) {
			return;
		}
		for (auto listener : mListeners) {
			listener->OnDeleteCommand(mThisPtr);
		}
	}

	SimpleDictCommand* mThisPtr;
	SimpleDictParam mParam;
	uint32_t mRefCount = 1;

	std::set<CommandUpdateListenerIF*> mListeners;
};

CString SimpleDictCommand::GetType() { return _T("SimpleDict"); }


SimpleDictCommand::SimpleDictCommand() : in(std::make_unique<PImpl>())
{
	in->mThisPtr = this;
	auto cmdRepo = soyokaze::core::CommandRepository::GetInstance();
	cmdRepo->RegisterListener(in.get());
}

SimpleDictCommand::~SimpleDictCommand()
{
	auto cmdRepo = soyokaze::core::CommandRepository::GetInstance();
	cmdRepo->UnregisterListener(in.get());
}

void SimpleDictCommand::AddListener(CommandUpdateListenerIF* listener)
{
	in->mListeners.insert(listener);
	listener->OnUpdateCommand(this);
}

void SimpleDictCommand::RemoveListener(CommandUpdateListenerIF* listener)
{
	in->mListeners.erase(listener);
}

const SimpleDictParam& SimpleDictCommand::GetParam()
{
	return in->mParam;
}

CString SimpleDictCommand::GetName()
{
	return in->mParam.mName;
}

CString SimpleDictCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString SimpleDictCommand::GetGuideString()
{
	// このコマンドはマッチしない
	return _T("");
}


CString SimpleDictCommand::GetTypeDisplayName()
{
	// コマンドとしてマッチしないが、キーワードマネージャに表示する文字列として使用する
	return _T("簡易辞書コマンド");
}

BOOL SimpleDictCommand::Execute(const Parameter& param)
{
	// このコマンドは直接実行しない
	return TRUE;
}

CString SimpleDictCommand::GetErrorString()
{
	return _T("");
}

HICON SimpleDictCommand::GetIcon()
{
	return IconLoader::Get()->LoadDefaultIcon();
}

int SimpleDictCommand::Match(Pattern* pattern)
{
	// このコマンドは直接実行させないのでつねにMismatchとする
	return Pattern::Mismatch;
}

bool SimpleDictCommand::IsEditable()
{
	return true;
}

int SimpleDictCommand::EditDialog(const Parameter*)
{
	// 設定変更画面を表示する
	SettingDialog dlg;
	dlg.SetParam(in->mParam);
	if (dlg.DoModal() != IDOK) {
		return 0;
	}

	// 変更後の設定値で上書き
	in->mParam = dlg.GetParam();

	// 名前の変更を登録しなおす
	auto cmdRepo = soyokaze::core::CommandRepository::GetInstance();
	cmdRepo->ReregisterCommand(this);

	// コマンドの設定情報の変更を通知
	for (auto listener : in->mListeners) {
		listener->OnUpdateCommand(this);
	}
	return 0;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool SimpleDictCommand::IsPriorityRankEnabled()
{
	return false;
}

soyokaze::core::Command*
SimpleDictCommand::Clone()
{
	auto clonedCmd = std::make_unique<SimpleDictCommand>();

	clonedCmd->in->mParam = in->mParam;
	clonedCmd->in->mListeners = in->mListeners;

	return clonedCmd.release();
}

bool SimpleDictCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	cmdFile->Set(entry, _T("description"), GetDescription());
	cmdFile->Set(entry, _T("FilePath"), in->mParam.mFilePath);
	cmdFile->Set(entry, _T("SheetName"), in->mParam.mSheetName);
	cmdFile->Set(entry, _T("Range"), in->mParam.mRangeFront);
	cmdFile->Set(entry, _T("RangeBack"), in->mParam.mRangeBack);
	cmdFile->Set(entry, _T("IsFirstRowHeader"), (bool)in->mParam.mIsFirstRowHeader);
	cmdFile->Set(entry, _T("IsMatchWithoutKeyword"), (bool)in->mParam.mIsMatchWithoutKeyword);
	cmdFile->Set(entry, _T("IsEnableReverse"), (bool)in->mParam.mIsEnableReverse);

	cmdFile->Set(entry, _T("aftertype"), in->mParam.mActionType);
	cmdFile->Set(entry, _T("aftercommand"), in->mParam.mAfterCommandName);
	cmdFile->Set(entry, _T("afterfilepath"), in->mParam.mAfterFilePath);
	cmdFile->Set(entry, _T("afterparam"), in->mParam.mAfterCommandParam);

	return true;
}

uint32_t SimpleDictCommand::AddRef()
{
	return ++in->mRefCount;
}

uint32_t SimpleDictCommand::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

bool SimpleDictCommand::NewDialog(
	const Parameter* param,
	SimpleDictCommand** newCmdPtr
)
{
	// パラメータ指定には対応していない
	// param;

	ExcelApplication app;
	if (app.IsInstalled() == false) {
		AfxMessageBox(_T("簡易辞書コマンドはExcelがインストールされていないと利用できません"));
		return false;
	}

	// 新規作成ダイアログを表示
	SettingDialog dlg;
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = dlg.GetParam();
	auto newCmd = std::make_unique<SimpleDictCommand>();
	newCmd->in->mParam = commandParam;

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

bool SimpleDictCommand::LoadFrom(CommandFile* cmdFile, void* e, SimpleDictCommand** newCmdPtr)
{
	ASSERT(newCmdPtr);

	CommandFile::Entry* entry = (CommandFile::Entry*)e;
	CString typeStr = cmdFile->Get(entry, _T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != SimpleDictCommand::GetType()) {
		return false;
	}

	CString name = cmdFile->GetName(entry);
	CString descriptionStr = cmdFile->Get(entry, _T("description"), _T(""));

	auto command = std::make_unique<SimpleDictCommand>();

	command->in->mParam.mName = name;
	command->in->mParam.mDescription = descriptionStr;

	command->in->mParam.mFilePath = cmdFile->Get(entry, _T("FilePath"), _T(""));
	command->in->mParam.mSheetName = cmdFile->Get(entry, _T("SheetName"), _T(""));
	command->in->mParam.mRangeFront = cmdFile->Get(entry, _T("Range"), _T(""));
	command->in->mParam.mRangeBack = cmdFile->Get(entry, _T("RangeBack"), _T(""));
	command->in->mParam.mIsFirstRowHeader = cmdFile->Get(entry, _T("IsFirstRowHeader"), false);
	command->in->mParam.mIsMatchWithoutKeyword = cmdFile->Get(entry, _T("IsMatchWithoutKeyword"), true);
	command->in->mParam.mIsEnableReverse = cmdFile->Get(entry, _T("IsEnableReverse"), false);

	command->in->mParam.mActionType = cmdFile->Get(entry, _T("aftertype"), 2);
	command->in->mParam.mAfterCommandName = cmdFile->Get(entry, _T("aftercommand"), _T(""));
	command->in->mParam.mAfterFilePath = cmdFile->Get(entry, _T("afterfilepath"), _T(""));
	command->in->mParam.mAfterCommandParam = cmdFile->Get(entry, _T("afterparam"), _T("$value"));

	if (newCmdPtr) {
		*newCmdPtr = command.release();
	}
	return true;
}

} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace soyokaze

