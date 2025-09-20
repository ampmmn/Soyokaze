#include "pch.h"
#include "ClipboardHistoryProvider.h"
#include "commands/clipboardhistory/ClipboardHistoryCommand.h"
#include "commands/clipboardhistory/ClipboardHistoryParam.h"
#include "commands/clipboardhistory/ClipboardHistoryDB.h"
#include "commands/clipboardhistory/ClipboardHistoryQueryCancellationToken.h"
#include "commands/clipboardhistory/ClipboardHistoryEventReceiver.h"
#include "commands/clipboardhistory/AppSettingClipboardHistoryPage.h"
#include "commands/clipboardhistory/ClipboardPreviewWindow.h"
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
	void OnAppExit() override {
		PreviewWindow::Get()->Destroy();
	}

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
		if (mIsInitialized) {
			// 初回呼び出し時に初期化とリロードを行う
			mReceiver.Initialize();
			Reload();
			mIsInitialized = false;
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
		mParam.Load((Settings&)pref->GetSettings());

		if (mParam.mIsEnable) {
			// クリップボード履歴を有効にする
			mHistoryDB.Load(mParam.mNumOfResults, mParam.mSizeLimit, mParam.mCountLimit);
			mHistoryDB.SetCancellationToken(&mCancelToken);
			mHistoryDB.UseRegExpSearch(mParam.mIsDisableMigemo == false);
			mReceiver.Activate(mParam.mInterval, mParam.mExcludePattern);
			mReceiver.AddListener(&mHistoryDB);
			PreviewWindow::Get()->SetEnable(mParam.mUsePreview);
		}
		else {
			// クリップボード履歴を無効にする
			mReceiver.Deactivate();
			mHistoryDB.Unload();
			PreviewWindow::Get()->Disable();
		}
	}

	Param mParam;
	bool mIsInitialized{true}; ///< 初期化済かどうか

	ClipboardHistoryDB mHistoryDB; ///< クリップボード履歴データベース
	QueryCancellationToken mCancelToken;   ///< キャンセル状態問い合わせ用オブジェクト
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
	if (in->mParam.mIsEnable == false) {
		return;
	}

	// プレフィックスが一致しない場合は抜ける
	const auto& prefix = in->mParam.mPrefix;
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

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t ClipboardHistoryProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(ClipboardHistoryCommand::TypeDisplayName());
	return 1;
}

} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp


