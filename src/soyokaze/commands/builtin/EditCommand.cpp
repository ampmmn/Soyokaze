#include "pch.h"
#include "framework.h"
#include "commands/builtin/EditCommand.h"
#include "commands/builtin/EditCandidateCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/core/CommandRepository.h"
#include "matcher/PartialMatchPattern.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

using Command = launcherapp::core::Command;

CString EditCommand::TYPE(_T("Builtin"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(EditCommand)

CString EditCommand::GetType()
{
	return TYPE;
}

EditCommand::EditCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("edit"))
{
	mDescription = _T("【編集】");
}

EditCommand::EditCommand(const EditCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

EditCommand::~EditCommand()
{
}

bool EditCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (BuiltinCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}
	if (ifid == IFID_EXTRACANDIDATESOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidateSource*)this;
		return true;
	}
	return false;
}

int EditCommand::Match(Pattern* pattern)
{
	int level = pattern->Match(GetName());
	if (level == Pattern::WholeMatch) {
		// 後続キーワードが存在する場合、追加候補(EditCandidateCommand)を優先させたいので、
		// edit自身は表示させない
		int numOfWords = pattern->GetWordCount();
		if (numOfWords > 1) {
			return Pattern::HiddenMatch;
		}
	}
	return level;

}


BOOL EditCommand::Execute(Parameter* param)
{
	auto cmdRepoPtr =
	 	launcherapp::core::CommandRepository::GetInstance();

	if (param->HasParameter() == false) {
		// キーワードマネージャを実行する
		cmdRepoPtr->ManagerDialog();
		return TRUE;
	}

	LPCTSTR editName = param->GetParam(0);
	RefPtr<launcherapp::core::Command> cmd(cmdRepoPtr->QueryAsWholeMatch(editName));

	if (cmd == nullptr) {
		CString msgStr((LPCTSTR)IDS_ERR_NAMEDOESNOTEXIST);
		msgStr += _T("\n\n");
		msgStr += editName;
		AfxMessageBox(msgStr);
		return TRUE;
	}

	constexpr bool isClone = false;
	cmdRepoPtr->EditCommandDialog(editName, isClone);
	return TRUE;
}

HICON EditCommand::GetIcon()
{
	return IconLoader::Get()->LoadEditIcon();
}

Command* EditCommand::Clone()
{
	return new EditCommand(*this);
}

/**
 	コマンドの候補として追加表示する項目を取得する
 	@return true:取得成功   false:取得失敗(表示しない)
 	@param[in]  pattern  入力パターン
 	@param[out] commands 表示する候補
*/
bool EditCommand::QueryCandidates(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	// コマンド名が一致しなければ候補を表示しない
	if (GetName().CompareNoCase(pattern->GetFirstWord()) != 0) {
		return false;
	}

	RefPtr<PatternInternal> pat2;
	if (pattern->QueryInterface(IFID_PATTERNINTERNAL, (void**)&pat2) == false) {
		return false;
	}

	std::vector<CString> words;
	CString queryStr;
	pat2->GetRawWords(words);
	for (size_t i = 1; i < words.size(); ++i) {
		queryStr += words[i];
		queryStr += _T(" ");
	}

	// "edit "の後に入力したキーワードのみで別のPartialMatchPatternを生成しておく
	RefPtr<PartialMatchPattern> patTmp(PartialMatchPattern::Create());
	patTmp->SetWholeText(queryStr);

	// すべてのコマンドを列挙
	std::vector<Command*> commandList;
	launcherapp::core::CommandRepository::GetInstance()->EnumCommands(commandList);

	// 列挙結果をアルファベット降順で表示するためにソートする
	std::sort(commandList.begin(), commandList.end(),
	          [](Command* l, Command* r) { return _tcscmp(l->GetName(), r->GetName()) < 0; });

	// 追加候補としてのコマンド(EditCandidateCommand)を生成する
	for (auto& cmd : commandList) {

		// 後続キーワードで絞込をおこない、該当するものを候補として表示する
		int level = cmd->Match(patTmp.get());
		if (level != Pattern::Mismatch) {
			commands.Add(CommandQueryItem(level, new EditCandidateCommand(cmd->GetName())));
		}
		cmd->Release();
	}

	return true;
}

/**
 	追加候補を表示するために内部でキャッシュしているものがあれば、それを削除する
*/
void EditCommand::ClearCache()
{
}


}
}
}
