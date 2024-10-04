#include "pch.h"
#include "SimpleDictCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/simple_dict/DictionaryLoader.h"
#include "commands/simple_dict/SimpleDictEditDialog.h"
#include "commands/simple_dict/SimpleDictionary.h"
#include "commands/simple_dict/SimpleDictAdhocCommand.h"
#include "commands/simple_dict/ExcelWrapper.h"
#include "commands/core/CommandRepository.h"
#include "mainwindow/LauncherWindowEventDispatcher.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/TimeoutChecker.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "commands/common/Message.h" // for PopupMessage
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "SharedHwnd.h"
#include <mutex>

namespace launcherapp {
namespace commands {
namespace simple_dict {

static bool GetLastUpdateTime(LPCTSTR path, FILETIME& ftime)
{
	if (PathIsURL(path)) {
		// URLパスは非対応(ここでチェックしなくても後段のCreateFileで失敗するはずではあるが..)
		return false;
	}

	HANDLE h = CreateFile(path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}
	GetFileTime(h, nullptr, nullptr, &ftime);
	CloseHandle(h);
	return true;
}

constexpr LPCTSTR TYPENAME = _T("SimpleDictCommand");

struct SimpleDictCommand::PImpl : public LauncherWindowEventListenerIF
{
	void OnLockScreenOccurred() override { }
	void OnUnlockScreenOccurred() override { }
	void OnTimer() override
	{
			if (mLastUpdate == 0) {
				GetLastUpdateTime(mParam.mFilePath, mLastUpdated);
				mLastUpdate = GetTickCount64(); 
			}

			if (GetTickCount64() - mLastUpdate <= 5000) {  // 更新チェック間隔は5秒に1回
				return;
			}

			if (shouldReload()) {
				// データ更新を予約する
				DictionaryLoader::Get()->AddWaitingQueue(mThisPtr);
				if (mParam.mIsNotifyUpdate) {
					// 更新を通知する
					CString msg;
					msg.Format(_T("【%s】ファイルが更新されました\n%s"), (LPCTSTR)mParam.mName, (LPCTSTR)mParam.mFilePath);
					launcherapp::commands::common::PopupMessage(msg);
				}
			}

			GetLastUpdateTime(mParam.mFilePath, mLastUpdated);
			mLastUpdate = GetTickCount64(); 
	}

	// 辞書データをリロードすべきか?
	bool shouldReload()
	{
		// ファイルがなければリロードしない
		if (PathFileExists(mParam.mFilePath) == FALSE) {
			return false;
		}

		// 更新日時が変化したらリロード
		FILETIME ft;
		return GetLastUpdateTime(mParam.mFilePath, ft) && memcmp(&ft, &mLastUpdated, sizeof(ft)) != 0;
	}

	bool QueryCandidates(Pattern* pattern, CommandQueryItemList& commands,utility::TimeoutChecker& tm);
	bool QueryCandidatesWithoutName(Pattern* pattern, CommandQueryItemList& commands,utility::TimeoutChecker& tm);
	int MatchRecord(Pattern* pattern, const Record& record, int offset);

	SimpleDictCommand* mThisPtr = nullptr;
	SimpleDictParam mParam;
	CommandHotKeyAttribute mHotKeyAttr;

	std::mutex mMutex;
	Dictionary mDictData;
	FILETIME mLastUpdated;
	uint64_t mLastUpdate = 0;
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
		commands.Add(CommandQueryItem(level, new SimpleDictAdhocCommand(mParam, record.mKey, record.mValue)));
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
		commands.Add(CommandQueryItem(level, new SimpleDictAdhocCommand(mParam, record.mKey, record.mValue)));
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
	LauncherWindowEventDispatcher::Get()->AddListener(in.get());
}

SimpleDictCommand::~SimpleDictCommand()
{
	LauncherWindowEventDispatcher::Get()->RemoveListener(in.get());
}

bool SimpleDictCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
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

/**
 * 種別を表す文字列を取得する
 * @return 文字列
 */
CString SimpleDictCommand::GetTypeName()
{
	return TYPENAME;
}

CString SimpleDictCommand::GetTypeDisplayName()
{
	// コマンドとしてマッチしないが、キーワードマネージャに表示する文字列として使用する
	return _T("簡易辞書コマンド");
}

BOOL SimpleDictCommand::Execute(const Parameter& param)
{
	UNREFERENCED_PARAMETER(param);

	// コマンド名単体(後続のパラメータなし)で実行したときは簡易辞書の候補一覧を列挙させる

	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 2, 1, 0);

	auto cmdline = GetName();
	cmdline += _T(" ");
	SendMessage(sharedWnd.GetHwnd(), WM_APP+11, 0, (LPARAM)(LPCTSTR)cmdline);
	return TRUE;
}

CString SimpleDictCommand::GetErrorString()
{
	return _T("");
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
		if (level == Pattern::FrontMatch) {
			return Pattern::FrontMatch;
		}
		if (level == Pattern::WholeMatch && pattern->GetWordCount() == 1) {
			// 入力欄からの入力で、前方一致するときは候補に出す
			return Pattern::WholeMatch;
		}
	}

	// 通常はこちら
	return Pattern::Mismatch;
}

int SimpleDictCommand::EditDialog(HWND parent)
{
	// 設定変更画面を表示する
	SettingDialog dlg(CWnd::FromHandle(parent));
	dlg.SetParam(in->mParam);
	dlg.SetHotKeyAttribute(in->mHotKeyAttr);

	if (dlg.DoModal() != IDOK) {
		return 1;
	}

	// 元の名前
	CString orgName = in->mParam.mName;

	// 変更後の設定値で上書き
	in->mParam = dlg.GetParam();
	dlg.GetHotKeyAttribute(in->mHotKeyAttr);

	// 名前の変更を登録しなおす
	auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
	cmdRepo->ReregisterCommand(this);

	// データ更新を予約する
	DictionaryLoader::Get()->AddWaitingQueue(this);

	return 0;
}

bool SimpleDictCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mHotKeyAttr;
	return true;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool SimpleDictCommand::IsPriorityRankEnabled()
{
	return true;
}

launcherapp::core::Command*
SimpleDictCommand::Clone()
{
	auto clonedCmd = std::make_unique<SimpleDictCommand>();

	clonedCmd->in->mParam = in->mParam;
	std::lock_guard<std::mutex> lock(in->mMutex);
	clonedCmd->in->mDictData = in->mDictData;

	return clonedCmd.release();
}

bool SimpleDictCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	entry->Set(_T("description"), GetDescription());
	entry->Set(_T("FilePath"), in->mParam.mFilePath);
	entry->Set(_T("SheetName"), in->mParam.mSheetName);
	entry->Set(_T("Range"), in->mParam.mRangeFront);
	entry->Set(_T("RangeBack"), in->mParam.mRangeBack);
	entry->Set(_T("IsFirstRowHeader"), (bool)in->mParam.mIsFirstRowHeader);
	entry->Set(_T("IsMatchWithoutKeyword"), (bool)in->mParam.mIsMatchWithoutKeyword);
	entry->Set(_T("IsEnableReverse"), (bool)in->mParam.mIsEnableReverse);
	entry->Set(_T("IsNotifyUpdate"), (bool)in->mParam.mIsNotifyUpdate);
	entry->Set(_T("IsExpandMacro"), (bool)in->mParam.mIsExpandMacro);
	entry->Set(_T("aftertype"), in->mParam.mActionType);
	entry->Set(_T("aftercommand"), in->mParam.mAfterCommandName);
	entry->Set(_T("afterfilepath"), in->mParam.mAfterFilePath);
	entry->Set(_T("afterparam"), in->mParam.mAfterCommandParam);

	return true;
}

bool SimpleDictCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != GetType()) {
		return false;
	}

	in->mParam.mName = entry->GetName();
	in->mParam.mDescription = entry->Get(_T("description"), _T(""));

	in->mParam.mFilePath = entry->Get(_T("FilePath"), _T(""));
	in->mParam.mSheetName = entry->Get(_T("SheetName"), _T(""));
	in->mParam.mRangeFront = entry->Get(_T("Range"), _T(""));
	in->mParam.mRangeBack = entry->Get(_T("RangeBack"), _T(""));
	in->mParam.mIsFirstRowHeader = entry->Get(_T("IsFirstRowHeader"), false);
	in->mParam.mIsMatchWithoutKeyword = entry->Get(_T("IsMatchWithoutKeyword"), true);
	in->mParam.mIsEnableReverse = entry->Get(_T("IsEnableReverse"), false);
	in->mParam.mIsNotifyUpdate = entry->Get(_T("IsNotifyUpdate"), false);
	in->mParam.mIsExpandMacro = entry->Get(_T("IsExpandMacro"), false);

	in->mParam.mActionType = entry->Get(_T("aftertype"), 2);
	in->mParam.mAfterCommandName = entry->Get(_T("aftercommand"), _T(""));
	in->mParam.mAfterFilePath = entry->Get(_T("afterfilepath"), _T(""));
	in->mParam.mAfterCommandParam = entry->Get(_T("afterparam"), _T("$value"));

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mHotKeyAttr); 

	// データ更新を予約する
	DictionaryLoader::Get()->AddWaitingQueue(this);

	return true;
}


bool SimpleDictCommand::NewDialog(
	const Parameter* param,
	SimpleDictCommand** newCmdPtr
)
{
	// パラメータ指定には対応していない
	UNREFERENCED_PARAMETER(param);

	ExcelApplication app;
	if (app.IsInstalled() == false) {
		AfxMessageBox(_T("簡易辞書コマンドを利用するにはExcelがインストールされている必要がありまず"));
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

	// データ更新を予約する
	DictionaryLoader::Get()->AddWaitingQueue(newCmd.get());

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


} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

