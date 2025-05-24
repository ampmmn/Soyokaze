#include "pch.h"
#include "ClipboardHistoryProvider.h"
#include "commands/clipboardhistory/ClipboardHistoryCommand.h"
#include "commands/clipboardhistory/ClipboardHistoryDB.h"
#include "commands/clipboardhistory/ClipboardHistoryEventReceiver.h"
#include "commands/clipboardhistory/AppSettingClipboardHistoryPage.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "commands/core/CommandFile.h"
#include "mainwindow/LauncherWindowEventDispatcher.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

using CommandRepository = launcherapp::core::CommandRepository;

/**
 * @brief ClipboardHistoryProvider の内部実装クラス
 */
struct ClipboardHistoryProvider::PImpl :
	public AppPreferenceListenerIF,
	public LauncherWindowEventListenerIF
{
	/**
	 * @brief コンストラクタ
	 */
	PImpl()
	{
		// アプリケーションの設定変更リスナーとして登録
		AppPreference::Get()->RegisterListener(this);
	}

	/**
	 * @brief デストラクタ
	 */
	virtual ~PImpl()
	{
		// アプリケーションの設定変更リスナーから解除
		AppPreference::Get()->UnregisterListener(this);
	}

	/**
	 * @brief アプリケーションの初回起動時に呼ばれる
	 */
	void OnAppFirstBoot() override {}

	/**
	 * @brief アプリケーションの通常起動時に呼ばれる
	 */
	void OnAppNormalBoot() override {
		// ランチャーウィンドウのイベントリスナーとして登録
		LauncherWindowEventDispatcher::Get()->AddListener(this);
	}

	/**
	 * @brief アプリケーションの設定が更新されたときに呼ばれる
	 */
	void OnAppPreferenceUpdated() override
	{
		// 設定をリロード
		Reload();
	}

	/**
	 * @brief アプリケーションの終了時に呼ばれる
	 */
	void OnAppExit() override {}

	/**
	 * @brief ロックスクリーンが発生したときに呼ばれる
	 */
	void OnLockScreenOccurred() override {}

	/**
	 * @brief ロックスクリーンが解除されたときに呼ばれる
	 */
	void OnUnlockScreenOccurred() override {}

	/**
	 * @brief タイマーイベントが発生したときに呼ばれる
	 */
	void OnTimer() override
	{
		if (mIsFirstCall) {
			// 初回呼び出し時に初期化とリロードを行う
			mReceiver.Initialize();
			Reload();
			mIsFirstCall = false;
		}
	}
	void OnLauncherActivate() override
	{
	}
	void OnLauncherUnactivate() override
	{
	}

	/**
	 * @brief 設定をリロードする
	 */
	void Reload()
	{
		auto pref = AppPreference::Get();
		mIsEnable = pref->IsEnableClipboardHistory();
		mPrefix = pref->GetClipboardHistoryPrefix();
		mNumOfResults = pref->GetClipboardHistoryNumberOfResults();
		mSizeLimit = pref->GetClipboardHistorySizeLimit();
		mCountLimit = pref->GetClipboardHistoryCountLimit();
		mInterval = pref->GetClipboardHistoryInterval();
		mExcludePattern = pref->GetClipboardHistoryExcludePattern();

		if (mIsEnable) {
			// クリップボード履歴を有効にする
			mHistoryDB.Load(mNumOfResults, mSizeLimit, mCountLimit);
			mHistoryDB.UseRegExpSearch(pref->IsDisableMigemoForClipboardHistory() == false);
			mReceiver.Activate(mInterval, mExcludePattern);
			mReceiver.AddListener(&mHistoryDB);
		}
		else {
			// クリップボード履歴を無効にする
			mReceiver.Deactivate();
			mHistoryDB.Unload();
		}
	}

	bool mIsEnable{false}; ///< クリップボード履歴が有効かどうか
	bool mIsFirstCall{true}; ///< 初回呼び出しかどうか

	CString mPrefix; ///< クリップボード履歴のプレフィックス
	int mNumOfResults{16}; ///< クリップボード履歴の結果数
	int mSizeLimit{64}; ///< クリップボード履歴のサイズ制限
	int mCountLimit{65536}; ///< クリップボード履歴のカウント制限
	int mInterval{500}; ///< クリップボード履歴のインターバル
	CString mExcludePattern; ///< クリップボード履歴の除外パターン

	ClipboardHistoryDB mHistoryDB; ///< クリップボード履歴データベース
	ClipboardHistoryEventReceiver mReceiver; ///< クリップボード履歴イベントレシーバー
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(ClipboardHistoryProvider)

/**
 * @brief コンストラクタ
 */
ClipboardHistoryProvider::ClipboardHistoryProvider() : in(new PImpl)
{
}

/**
 * @brief デストラクタ
 */
ClipboardHistoryProvider::~ClipboardHistoryProvider()
{
}

/**
 * @brief プロバイダの名前を取得する
 * @return プロバイダの名前
 */
CString ClipboardHistoryProvider::GetName()
{
	return _T("ClipboardHistoryCommandProvider");
}

/**
 * @brief 一時的なコマンドを必要に応じて提供する
 * @param pattern パターン
 * @param commands コマンドリスト
 */
void ClipboardHistoryProvider::QueryAdhocCommands(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	// 機能を利用しない場合は抜ける
	if (in->mIsEnable == false) {
		return;
	}

	// プレフィックスが一致しない場合は抜ける
	const auto& prefix = in->mPrefix;
	if (prefix.IsEmpty() == FALSE && prefix.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return;
	}

	// クリップボード履歴をクエリ
	ClipboardHistoryDB::ResultList result;
	in->mHistoryDB.Query(pattern, result);

	if (result.empty()) {
		// 件数0件の場合でも、弱一致の候補表示を抑制するためにダミーの項目を追加する
		commands.Add(CommandQueryItem(Pattern::HiddenMatch, 
					new ClipboardHistoryCommand(prefix, 0, _T(""))));
		return;
	}

	// 結果をコマンドリストに追加
	for (auto& item : result) {
		// 最低でも前方一致扱いとする
		int level = (std::max)(item.mMatchLevel, (int)Pattern::FrontMatch);
		commands.Add(CommandQueryItem(level, new ClipboardHistoryCommand(prefix, item.mAppendTime, item.mData)));
	}
}

} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp


