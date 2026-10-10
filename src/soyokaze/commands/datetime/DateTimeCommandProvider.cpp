#include "pch.h"
#include "DateTimeCommandProvider.h"
#include "commands/datetime/DateTimeCommand.h"
#include "commands/datetime/DateTimeValueCommand.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace datetime {


using CommandRepository = launcherapp::core::CommandRepository;

// 日数オフセットの上限(約100年)
static const int MAX_OFFSET_DAYS = 36500;

struct DateTimeCommandProvider::PImpl :
	public AppPreferenceListenerIF
{
	PImpl()
	{
		// 設定変更を受け取るためリスナーとして登録する
		AppPreference::Get()->RegisterListener(this, _T("DateTimeCommandProvider"));
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	// AppPreferenceListenerIF
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		// 設定が変更されたら値を取得し直す
		Load();
	}
	void OnAppExit() override {}

	// 設定値をAppPreferenceから取得してキャッシュする
	void Load()
	{
		auto pref = AppPreference::Get();
		mIsEnable = pref->IsEnableDateTime();
		mPrefix = pref->GetDateTimePrefix();
	}

	// 日時関連の機能を使用するか
	bool mIsEnable{false};
	// 現在日時を表示するプレフィックス
	CString mPrefix;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(DateTimeCommandProvider)


DateTimeCommandProvider::DateTimeCommandProvider() : in(std::make_unique<PImpl>())
{
}

DateTimeCommandProvider::~DateTimeCommandProvider()
{
}

CString DateTimeCommandProvider::GetName()
{
	return _T("DateTime");
}

// 設定値を取得する
void DateTimeCommandProvider::PrepareAdhocCommands()
{
	in->Load();
}

/**
  時間差の入力(HH:MM-HH:MM)を解析する
	@return true:解析成功  false:形式が一致しない、または時刻の範囲外
	@param[in]  text   入力文字列
	@param[out] result 時間差(開始時刻 - 終了時刻)
*/
bool DateTimeCommandProvider::ParseTimeSpan(const CString& text, CTimeSpan& result)
{
	tstring str(text);

	// HH:MM-HH:MM の形式に一致するか確認する
	static tregex regTime1(_T("^ *([0-2][0-9]):([0-5][0-9]) *- *([0-2][0-9]):([0-5][0-9]) *$"));
	std::match_results<tstring::const_iterator> m;
	if (std::regex_match(str, m, regTime1) == false) {
		return false;
	}

	// 時・分を取り出す
	int h1 = std::stoi(m[1].str());
	int m1 = std::stoi(m[2].str());
	int h2 = std::stoi(m[3].str());
	int m2 = std::stoi(m[4].str());

	// 24時以上は無効とする
	if (h1 >= 24 || h2 >= 24) {
		return false;
	}

	CTimeSpan ts1(0, h1, m1, 0);
	CTimeSpan ts2(0, h2, m2, 0);
	result = ts1 - ts2;
	return true;
}

/**
  日数オフセットの入力(N day(s) later|ago, N日前|後)を解析する
	@return true:解析成功  false:形式が一致しない
	@param[in]  text 入力文字列
	@param[out] days 日数オフセット(後は正、前は負)
*/
bool DateTimeCommandProvider::ParseDayOffset(const CString& text, int& days)
{
	tstring str(text);

	// 英語形式(大文字小文字は区別しない)
	static tregex regEn(_T("^ *([0-9]{1,6}) +days? +(later|ago) *$"), std::regex_constants::icase);
	// 日本語形式
	static tregex regJa(_T("^ *([0-9]{1,6}) *日 *(前|後) *$"));

	std::match_results<tstring::const_iterator> m;
	if (std::regex_match(str, m, regEn)) {
		int n = std::stoi(m[1].str());
		// 上限を超える日数は扱わない
		if (n > MAX_OFFSET_DAYS) {
			return false;
		}
		// later は後(正)、ago は前(負)
		days = (_tcsicmp(m[2].str().c_str(), _T("later")) == 0) ? n : -n;
		return true;
	}
	if (std::regex_match(str, m, regJa)) {
		int n = std::stoi(m[1].str());
		// 上限を超える日数は扱わない
		if (n > MAX_OFFSET_DAYS) {
			return false;
		}
		// 後(正)、前(負)
		days = (m[2].str() == _T("後")) ? n : -n;
		return true;
	}
	return false;
}

/**
  日時を指定された種別の文字列に変換する
	@return 変換後の文字列
	@param[in] t    変換対象の日時
	@param[in] kind 変換する種別
*/
CString DateTimeCommandProvider::FormatDateTime(const CTime& t, DateTimeKind kind)
{
	switch (kind) {
	case DateTimeKind::DateTime:
		return t.Format(_T("%Y/%m/%d %H:%M"));
	case DateTimeKind::Date:
		return t.Format(_T("%Y/%m/%d"));
	case DateTimeKind::Time:
		return t.Format(_T("%H:%M"));
	case DateTimeKind::Weekday: {
		// GetDayOfWeek は 1(日)～7(土) を返すため、添字に変換する
		static const TCHAR* WEEKDAYS[] = { _T("日"), _T("月"), _T("火"), _T("水"), _T("木"), _T("金"), _T("土") };
		return CString(WEEKDAYS[t.GetDayOfWeek() - 1]) + _T("曜日");
	}
	}
	return _T("");
}

/**
  基準日時から、日時候補(日時/日付/時刻/曜日)の表示文字列と種別名の組を作成する
	@return 候補の一覧(表示文字列, 種別名)
	@param[in] base 基準日時
*/
std::vector<std::pair<CString, CString>> DateTimeCommandProvider::MakeDateTimeCandidates(const CTime& base)
{
	std::vector<std::pair<CString, CString>> candidates;
	candidates.emplace_back(FormatDateTime(base, DateTimeKind::DateTime), _T("日時"));
	candidates.emplace_back(FormatDateTime(base, DateTimeKind::Date), _T("日付"));
	candidates.emplace_back(FormatDateTime(base, DateTimeKind::Time), _T("時刻"));
	candidates.emplace_back(FormatDateTime(base, DateTimeKind::Weekday), _T("曜日"));
	return candidates;
}

/**
  現在日時に日数オフセットを加えた日時の候補を、コマンド一覧に追加する
	@param[out] commands 候補を追加する一覧
	@param[in]  days     日数オフセット(プレフィックス入力時は0)
*/
void DateTimeCommandProvider::AddDateTimeCandidates(CommandQueryItemList& commands, int days)
{
	// 現在時刻を起点に日数を加算する
	CTime base = CTime::GetCurrentTime() + CTimeSpan(days, 0, 0, 0);
	// 1970年より前は CTime で扱えないため候補を出さない
	if (base.GetTime() < 0) {
		return;
	}

	auto candidates = MakeDateTimeCandidates(base);
	for (const auto& candidate : candidates) {
		commands.Add(CommandQueryItem(Pattern::WholeMatch, new DateTimeValueCommand(candidate.first, candidate.second)));
	}
}

// 一時的なコマンドを必要に応じて提供する
void DateTimeCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	CString cmdline = pattern->GetWholeString();

	cmdline = cmdline.Trim();
	if (cmdline.IsEmpty()) {
		return;
	}

	// 日時関連の機能が無効なら何もしない
	if (in->mIsEnable == false) {
		return;
	}

	// 時間差(HH:MM-HH:MM)の入力: プレフィックス不要
	CTimeSpan result;
	if (ParseTimeSpan(cmdline, result)) {
		commands.Add(CommandQueryItem(Pattern::WholeMatch, new DateTimeCommand(result, TYPE_HOUR)));
		commands.Add(CommandQueryItem(Pattern::WholeMatch, new DateTimeCommand(result, TYPE_MINUTE)));
		commands.Add(CommandQueryItem(Pattern::WholeMatch, new DateTimeCommand(result, TYPE_SECOND)));
		return;
	}

	// 日数オフセットの入力(N day(s) later|ago, N日前|後): プレフィックス不要
	int days = 0;
	if (ParseDayOffset(cmdline, days)) {
		// 解析した日数分のオフセットで、プレフィックス入力時と同じ候補を表示する
		AddDateTimeCandidates(commands, days);
		return;
	}

	// プレフィックスのみが入力された場合は、オフセット0日として候補を表示する
	if (in->mPrefix.IsEmpty() || cmdline.CompareNoCase(in->mPrefix) != 0) {
		return;
	}
	AddDateTimeCandidates(commands, 0);
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t DateTimeCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(DateTimeCommand::TypeDisplayName());
	displayNames.push_back(_T("日時"));
	displayNames.push_back(_T("日付"));
	displayNames.push_back(_T("時刻"));
	displayNames.push_back(_T("曜日"));
	return 5;
}


} // end of namespace datetime
} // end of namespace commands
} // end of namespace launcherapp

