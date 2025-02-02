#include "pch.h"
#include "framework.h"
#include "DeleteCommand.h"
#include "commands/builtin/DeleteCandidateCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "matcher/PartialMatchPattern.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

CString DeleteCommand::TYPE(_T("Builtin-Delete"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(DeleteCommand)

CString DeleteCommand::GetType()
{
	return TYPE;
}

DeleteCommand::DeleteCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("delete"))
{
	mDescription = _T("【コマンドを削除】");
}

DeleteCommand::DeleteCommand(const DeleteCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

DeleteCommand::~DeleteCommand()
{
}

bool DeleteCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (ifid == IFID_EXTRACANDIDATESOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidateSource*)this;
		return true;
	}
	return false;
}

int DeleteCommand::Match(Pattern* pattern)
{
	int level = pattern->Match(GetName());
	if (level == Pattern::WholeMatch) {
		// 後続キーワードが存在する場合、追加候補(DeleteCandidateCommand)を優先させたいので、
		// delete自身は表示させない
		int numOfWords = pattern->GetWordCount();
		if (numOfWords > 1) {
			return Pattern::HiddenMatch;
		}
	}
	return level;

}


BOOL DeleteCommand::Execute(Parameter* param)
{
	if (param->HasParameter() == false) {
		AfxMessageBox(IDS_ERR_NODELETECOMMAND);
		return TRUE;
	}

	LPCTSTR delName = param->GetParam(0);

	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();

	auto cmd = cmdRepoPtr->QueryAsWholeMatch(delName);
	if (cmd == nullptr) {
		CString msg;
		msg.Format(IDS_ERR_COMMANDDOESNOTEXIST);
		msg += _T("\n");
		msg+= delName;
		AfxMessageBox(msg);
		return TRUE;
	}

	// Note: 現状はシステムコマンド作成をサポートしていないので削除を許可しない
	RefPtr<Editable> editable;
	if (cmd->QueryInterface(IFID_EDITABLE, (void**)&editable) == false) {
		return false;
	}
	if (editable->IsDeletable() == false) {
		return TRUE;
	}

	// 削除前の確認
	CString confirmMsg((LPCTSTR)IDS_CONFIRM_DELETE);
	confirmMsg += _T("\n");
	confirmMsg += _T("\n");
	confirmMsg += delName;

	int sel = AfxMessageBox(confirmMsg, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
	if (sel != IDYES) {
		return TRUE;
	}

	cmdRepoPtr->UnregisterCommand(cmd);
	return TRUE;
}

HICON DeleteCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-5383);
}

launcherapp::core::Command* DeleteCommand::Clone()
{
	return new DeleteCommand(*this);
}

/**
 	コマンドの候補として追加表示する項目を取得する
 	@return true:取得成功   false:取得失敗(表示しない)
 	@param[in]  pattern  入力パターン
 	@param[out] commands 表示する候補
*/
bool DeleteCommand::QueryCandidates(
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

	// "delete "の後に入力したキーワードのみで別のPartialMatchPatternを生成しておく
	RefPtr<PartialMatchPattern> patTmp(PartialMatchPattern::Create());
	patTmp->SetWholeText(queryStr);

	// すべてのコマンドを列挙
	std::vector<Command*> commandList;
	launcherapp::core::CommandRepository::GetInstance()->EnumCommands(commandList);

	// 列挙結果をアルファベット降順で表示するためにソートする
	std::sort(commandList.begin(), commandList.end(),
	          [](Command* l, Command* r) { return _tcscmp(l->GetName(), r->GetName()) < 0; });

	// 追加候補としてのコマンド(DeleteCandidateCommand)を生成する
	for (auto& cmd : commandList) {

		// 後続キーワードで絞込をおこない、該当するものを候補として表示する
		int level = cmd->Match(patTmp.get());
		if (level != Pattern::Mismatch) {
			commands.Add(CommandQueryItem(level, new DeleteCandidateCommand(cmd->GetName())));
		}
		cmd->Release();
	}

	return true;
}

/**
 	追加候補を表示するために内部でキャッシュしているものがあれば、それを削除する
*/
void DeleteCommand::ClearCache()
{
}


}
}
}
