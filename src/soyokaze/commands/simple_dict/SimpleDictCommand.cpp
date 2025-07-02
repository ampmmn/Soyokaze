#include "pch.h"
#include "SimpleDictCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/simple_dict/DictionaryLoader.h"
#include "commands/simple_dict/SimpleDictCommandEditor.h"
#include "commands/simple_dict/SimpleDictionary.h"
#include "commands/simple_dict/SimpleDictAdhocCommand.h"
#include "commands/simple_dict/ExcelWrapper.h"
#include "commands/common/ExecutablePath.h"
#include "commands/core/CommandRepository.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/TimeoutChecker.h"
#include "utility/LocalDirectoryWatcher.h"
#include "utility/Path.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "commands/common/Message.h" // for PopupMessage
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "mainwindow/controller/MainWindowController.h"
#include <mutex>

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace simple_dict {

struct SimpleDictCommand::PImpl
{
	void Reload()
	{
		spdlog::info(_T("SimpleDictCommand: reload db. command name:{}"), (LPCTSTR)mParam.mName);

	 	// データ更新を予約する
		DictionaryLoader::Get()->AddWaitingQueue(mThisPtr);

		// 更新を通知する
		if (mParam.mIsNotifyUpdate) {
			CString msg;
			msg.Format(_T("【%s】ファイルが更新されました\n%s"), (LPCTSTR)mParam.mName, (LPCTSTR)mParam.mFilePath);
			PopupMessage(msg);
		}
	}

	bool QueryCandidates(Pattern* pattern, CommandQueryItemList& commands, utility::TimeoutChecker& tm);
	bool QueryCandidatesWithoutName(Pattern* pattern, CommandQueryItemList& commands, utility::TimeoutChecker& tm);
	int MatchRecord(Pattern* pattern, const Record& record, int offset);

	SimpleDictCommand* mThisPtr{nullptr};
	SimpleDictParam mParam;

	std::mutex mMutex;
	Dictionary mDictData;
	CString mErrMsg;
	uint32_t mWatcherId{0};
};

/**
 	検索処理(コマンド名が入力していなければ候補を表示しない)
 	@return        true: 検索結果あり   false:検索結果なし
 	@param[in]     pattern  検索パターン
 	@param[out]    commands 検索して見つかった候補を入れる入れ物
 	@param[in]     tm       タイムアウト判定用
*/
bool SimpleDictCommand::PImpl::QueryCandidates(
	Pattern* pattern,
	CommandQueryItemList& commands,
	utility::TimeoutChecker& tm
)
{
	if (mParam.mName.CompareNoCase(pattern->GetFirstWord()) != 0) {
		// コマンド名が一致しない場合は検索対象外
		return false;
	}
	// 実行する処理の種別が「プログラム実行」で、そのパスが存在しない場合は追加の候補を出さない
	// (どうせ実行できないので)
	if (mParam.mActionType == 1) {
		ExecutablePath path(mParam.mAfterFilePath);
		if (path.IsExecutable() == false) {
			return false;
		}
	}

	int hitCount = 0;
	int limit = 20;

	// 辞書データをひとつずつ比較する
	std::lock_guard<std::mutex> lock(mMutex);
	for (const auto& record : mDictData.mRecords) {

		if (tm.IsTimeout()) {
			// 一定時間たっても終わらなかったらあきらめる
			break;
		}

		int level = MatchRecord(pattern, record, 1);
		if (level == Pattern::Mismatch) {
			continue;
		}

		if (level == Pattern::PartialMatch) {
			// 最低でも前方一致扱いにする(先頭のコマンド名は合致しているため)
			level = Pattern::FrontMatch;
		}
		commands.Add(CommandQueryItem(level, new SimpleDictAdhocCommand(mParam, record)));
		if (++hitCount >= limit) {
			break;
		}
	}
	return hitCount > 0;
}

/**
 	検索処理(コマンド名が入力していなくても候補を表示する、の場合)
 	@return        true: 検索結果あり   false:検索結果なし
 	@param[in]     pattern  検索パターン
 	@param[out]    commands 検索して見つかった候補を入れる入れ物
 	@param[in]     tm       タイムアウト判定用
*/
bool SimpleDictCommand::PImpl::QueryCandidatesWithoutName(
	Pattern* pattern,
	CommandQueryItemList& commands,
	utility::TimeoutChecker& tm
)
{
	// 実行する処理の種別が「プログラム実行」で、そのパスが存在しない場合は追加の候補を出さない
	// (どうせ実行できないので)
	if (mParam.mActionType == 1) {
		ExecutablePath path(mParam.mAfterFilePath);
		if (path.IsExecutable() == false) {
			return false;
		}
	}

	int hitCount = 0;
	int limit = 20;

	// 辞書データをひとつずつ比較する
	std::lock_guard<std::mutex> lock(mMutex);
	for (const auto& record : mDictData.mRecords) {

		if (tm.IsTimeout()) {
			// 一定時間たっても終わらなかったらあきらめる
			break;
		}

		int level = Pattern::Mismatch;
		if (mParam.mName.CompareNoCase(pattern->GetFirstWord()) != 0) {
			// コマンド名が一致しない場合。コマンド名がなくても検索する。
			level = MatchRecord(pattern, record, 0);

			if (level != Pattern::Mismatch) {
				// コマンド名なしで候補を表示する場合は弱一致扱いとする
				level = Pattern::WeakMatch;
			}
		}
		else {
			// コマンド名が一致する場合。コマンド名を除いて検索する
			level = MatchRecord(pattern, record, 1);
			if (level == Pattern::PartialMatch) {
				// 最低でも前方一致扱いにする(先頭のコマンド名は合致しているため)
				level = Pattern::FrontMatch;
			}
		}

		if (level == Pattern::Mismatch) {
			continue;
		}
		commands.Add(CommandQueryItem(level, new SimpleDictAdhocCommand(mParam, record)));
		if (++hitCount >= limit) {
			break;
		}
	}

	return hitCount > 0;
}

int SimpleDictCommand::PImpl::MatchRecord(Pattern* pattern, const Record& record, int offset)
{
	int level = pattern->Match(record.mKey, offset);
	if (level != Pattern::Mismatch) {
		return level;
	}
	if (mParam.mIsEnableReverse == false) {
		return Pattern::Mismatch;
	}

	// 逆引きが有効なら値でのマッチングも行う
	return pattern->Match(record.mValue, offset);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CString SimpleDictCommand::GetType() { return _T("SimpleDict"); }


SimpleDictCommand::SimpleDictCommand() : in(std::make_unique<PImpl>())
{
	in->mThisPtr = this;
}

SimpleDictCommand::~SimpleDictCommand()
{
}

bool SimpleDictCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (UserCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}
	if (ifid == IFID_EXTRACANDIDATESOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidateSource*)this;
		return true;
	}
	return false;
}


void SimpleDictCommand::UpdateDictionary(Dictionary& dict)
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	in->mDictData.mRecords.swap(dict.mRecords);
}

void SimpleDictCommand::SetParam(const SimpleDictParam& param)
{
	in->mParam = param;
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
	return _T("キーワード入力すると候補を絞り込むことができます");
}

CString SimpleDictCommand::GetTypeDisplayName()
{
	// コマンドとしてマッチしないが、キーワードマネージャに表示する文字列として使用する
	return TypeDisplayName();
}

bool SimpleDictCommand::CanExecute()
{
	if (in->mParam.mActionType == 1) {
		// mAfterFilePathに$key $value($value2) を含む場合、選択するまで結果が確定しないため、
		// リンク切れチェックはできない
		const auto& filePath = in->mParam.mAfterFilePath;

		bool hasKey = filePath.Find(_T("$key")) != -1;
		bool hasValue = filePath.Find(_T("$value")) != -1;

		ExecutablePath path(filePath);
		if (hasKey == false && hasValue == false && path.IsExecutable() == false) {
			in->mErrMsg = _T("！リンク切れ！");
			return false;
		}
	}
	return true;
}


BOOL SimpleDictCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// コマンド名単体(後続のパラメータなし)で実行したときは簡易辞書の候補一覧を列挙させる

	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	bool isShowToggle = false;
	mainWnd->ActivateWindow(isShowToggle);

	auto cmdline = GetName();
	cmdline += _T(" ");
	mainWnd->SetText(cmdline);

	return TRUE;
}

CString SimpleDictCommand::GetErrorString()
{
	return in->mErrMsg;
}

HICON SimpleDictCommand::GetIcon()
{
	// バインダーに矢印みたいなアイコン
	return IconLoader::Get()->GetImageResIcon(-5301);
}

int SimpleDictCommand::Match(Pattern* pattern)
{
	if (pattern->shouldWholeMatch() && pattern->Match(GetName()) == Pattern::WholeMatch) {
		// 内部のコマンド名マッチング用の判定
		return Pattern::WholeMatch;
	}
	else if (pattern->shouldWholeMatch() == false) {
		int level = pattern->Match(GetName());
		if (level == Pattern::FrontMatch || level == Pattern::PartialMatch) {
			return level;
		}

		if (level == Pattern::WholeMatch) {
			// 後続のキーワードが存在する場合は非表示
			return (pattern->GetWordCount() == 1) ? Pattern::WholeMatch : Pattern::HiddenMatch;
		}
	}

	// 通常はこちら
	return Pattern::Mismatch;
}

bool SimpleDictCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
SimpleDictCommand::Clone()
{
	auto clonedCmd = make_refptr<SimpleDictCommand>();

	clonedCmd->in->mParam = in->mParam;
	std::lock_guard<std::mutex> lock(in->mMutex);
	clonedCmd->in->mDictData = in->mDictData;

	return clonedCmd.release();
}

bool SimpleDictCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);
	entry->Set(_T("Type"), GetType());
	return in->mParam.Save(entry);
}

bool SimpleDictCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != GetType()) {
		return false;
	}

	SimpleDictParam paramTmp;
	paramTmp.Load(entry);

	if (paramTmp == in->mParam) {
		// 変化がなければ何もしない
		SPDLOG_DEBUG(_T("skip loading"));
		return true;
	}

	in->mParam.swap(paramTmp);

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mParam.mHotKeyAttr); 

	// データ更新を予約する
	ReserveUpdate();

	return true;
}


bool SimpleDictCommand::NewDialog(
	Parameter* param,
	SimpleDictCommand** newCmdPtr
)
{
	// パラメータ指定には対応していない
	UNREFERENCED_PARAMETER(param);

	if (ExcelApplication::IsInstalled() == false) {
		AfxMessageBox(_T("簡易辞書コマンドを利用するにはExcelがインストールされている必要がありまず"));
		return false;
	}

	// 新規作成ダイアログを表示
	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = cmdEditor->GetParam();
	auto newCmd = make_refptr<SimpleDictCommand>();
	newCmd->in->mParam = commandParam;

	// データ更新を予約する
	newCmd->ReserveUpdate();

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

// コマンドを編集するためのダイアログを作成/取得する
bool SimpleDictCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
{
	if (editor == nullptr) {
		return false;
	}

	auto cmdEditor = new CommandEditor(CWnd::FromHandle(parent));
	cmdEditor->SetParam(in->mParam);

	*editor = cmdEditor;
	return true;
}

// ダイアログ上での編集結果をコマンドに適用する
bool SimpleDictCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_SIMPLEDICTCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	auto param = cmdEditor->GetParam();
	bool isPathChanged = in->mParam.mFilePath != param.mFilePath;

	in->mParam = param;

	// データ更新を予約する
	ReserveUpdate(isPathChanged);

	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool SimpleDictCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_SIMPLEDICTCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<SimpleDictCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	// データ更新を予約する
	newCmd->ReserveUpdate();

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}


/**
 	コマンドの候補として追加表示する項目を取得する
 	@return true:取得成功   false:取得失敗(表示しない)
 	@param[in]  pattern  入力パターン
 	@param[out] commands 表示する候補
*/
bool SimpleDictCommand::QueryCandidates(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	utility::TimeoutChecker tm(100);  // 100msecでタイムアウト

	if (in->mParam.mIsMatchWithoutKeyword == false) {
		return in->QueryCandidates(pattern, commands, tm);
	}
	else {
		return in->QueryCandidatesWithoutName(pattern, commands, tm);
	}
}

/**
 	追加候補を表示するために内部でキャッシュしているものがあれば、それを削除する
*/
void SimpleDictCommand::ClearCache()
{
}


void SimpleDictCommand::ReserveUpdate(bool isResiterWatcher)
{
	// データ更新を予約する
	DictionaryLoader::Get()->AddWaitingQueue(this);

	if (isResiterWatcher == false) {
		return;
	}

	// ファイル変更通知を受け取るための登録
	if (in->mWatcherId != 0) {
		LocalDirectoryWatcher::GetInstance()->Unregister(in->mWatcherId);
		in->mWatcherId = 0;
	}

	in->mWatcherId = 
		LocalDirectoryWatcher::GetInstance()->Register(in->mParam.mFilePath, [](void* p) {
		auto thisPtr = (PImpl*)p;
		thisPtr->Reload();
	}, in.get());
}

CString SimpleDictCommand::TypeDisplayName()
{
	return _T("簡易辞書コマンド");
}


} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

