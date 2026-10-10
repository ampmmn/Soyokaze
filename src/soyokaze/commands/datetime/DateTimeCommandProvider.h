#pragma once

#include "commands/common/AdhocCommandProviderBase.h"
#include "commands/core/CommandProviderIF.h"


namespace launcherapp {
namespace commands {
namespace datetime {

// 日時候補の種別
enum class DateTimeKind {
	DateTime,	// 日付と時刻(例: 2026/10/10 14:17)
	Date,		// 日付(例: 2026/10/10)
	Time,		// 時刻(例: 14:17)
	Weekday,	// 曜日(例: 土曜日)
};

/**
  日時に関するコマンドを提供するProvider
	- 時間差(HH:MM-HH:MM)の計算
	- 日数オフセット(N day(s) later|ago, N日前|後)による日時の計算
	- プレフィックス入力時の現在日時の表示
	の機能を持つ。有効/無効は設定(拡張機能)に従う。
*/
class DateTimeCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	DateTimeCommandProvider();
	virtual ~DateTimeCommandProvider();

public:
	virtual CString GetName();

	// 設定値を取得する
	void PrepareAdhocCommands() override;
	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);
	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	/**
	  時間差の入力(HH:MM-HH:MM)を解析する
		@return true:解析成功  false:形式が一致しない、または時刻の範囲外
		@param[in]  text   入力文字列
		@param[out] result 時間差(開始時刻 - 終了時刻)
	*/
	static bool ParseTimeSpan(const CString& text, CTimeSpan& result);

	/**
	  日数オフセットの入力(N day(s) later|ago, N日前|後)を解析する
		@return true:解析成功  false:形式が一致しない
		@param[in]  text 入力文字列
		@param[out] days 日数オフセット(後は正、前は負)
	*/
	static bool ParseDayOffset(const CString& text, int& days);

	/**
	  日時を指定された種別の文字列に変換する
		@return 変換後の文字列
		@param[in] t    変換対象の日時
		@param[in] kind 変換する種別
	*/
	static CString FormatDateTime(const CTime& t, DateTimeKind kind);

	/**
	  基準日時から、日時候補(日時/日付/時刻/曜日)の表示文字列と種別名の組を作成する
		@return 候補の一覧(表示文字列, 種別名)
		@param[in] base 基準日時
	*/
	static std::vector<std::pair<CString, CString>> MakeDateTimeCandidates(const CTime& base);

	/**
	  現在日時に日数オフセットを加えた日時の候補を、コマンド一覧に追加する
		@param[out] commands 候補を追加する一覧
		@param[in]  days     日数オフセット(プレフィックス入力時は0)
	*/
	static void AddDateTimeCandidates(CommandQueryItemList& commands, int days);


	DECLARE_COMMANDPROVIDER(DateTimeCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace datetime
} // end of namespace commands
} // end of namespace launcherapp

