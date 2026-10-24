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

namespace {

/**
  指定された年月日と時刻をCTimeに変換する
	@return true:変換成功  false:日付または時刻が範囲外
	@param[in]  year   年
	@param[in]  month  月
	@param[in]  day    日
	@param[in]  hour   時
	@param[in]  minute 分
	@param[out] result 変換後の日時
*/
bool MakeDateTime(int year, int month, int day, int hour, int minute, CTime& result)
{
	// CTimeで扱える年と時刻の範囲を確認する
	if (year < 1970 || year > 3000 || hour < 0 || hour >= 24 || minute < 0 || minute >= 60) {
		return false;
	}
	if (month < 1 || month > 12 || day < 1) {
		return false;
	}

	static const int MONTH_DAYS[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int maxDay = MONTH_DAYS[month - 1];
	bool isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	if (month == 2 && isLeapYear) {
		maxDay = 29;
	}
	// 平年の2/29は3/1として扱う
	if (month == 2 && day == 29 && isLeapYear == false) {
		month = 3;
		day = 1;
		maxDay = 31;
	}
	if (day > maxDay) {
		return false;
	}

	result = CTime(year, month, day, hour, minute, 0);
	return true;
}

/**
  時間差の片側を日時として解析する
	@return true:解析成功  false:形式が一致しない、または日時の範囲外
	@param[in]  text     入力文字列
	@param[in]  baseTime 現在日時および年省略時の基準日時
	@param[in]  prefix   nowの別名として扱う設定プレフィックス
	@param[out] result   解析した日時
*/
bool ParseDateTimeOperand(const CString& text, const CTime& baseTime, const CString& prefix, CTime& result)
{
	CString operand(text);
	operand.Trim();
	if (operand.CompareNoCase(_T("now")) == 0 || (prefix.IsEmpty() == false && operand.CompareNoCase(prefix) == 0)) {
		result = baseTime;
		return true;
	}

	tstring str(operand);
	std::match_results<tstring::const_iterator> m;
	static tregex regDateTimeHyphen(_T("^([0-9]{4})-([0-9]{2})-([0-9]{2}) +([0-2][0-9]):([0-5][0-9])$"));
	static tregex regDateTimeSlash(_T("^([0-9]{4})/([0-9]{2})/([0-9]{2}) +([0-2][0-9]):([0-5][0-9])$"));
	static tregex regDateTimeNoYear(_T("^([0-9]{1,2})/([0-9]{1,2}) +([0-2][0-9]):([0-5][0-9])$"));
	static tregex regTime(_T("^([0-2][0-9]):([0-5][0-9])$"));

	int year = baseTime.GetYear();
	int month = 0;
	int day = 0;
	int hour = 0;
	int minute = 0;
	if (std::regex_match(str, m, regDateTimeHyphen) || std::regex_match(str, m, regDateTimeSlash)) {
		year = std::stoi(m[1].str());
		month = std::stoi(m[2].str());
		day = std::stoi(m[3].str());
		hour = std::stoi(m[4].str());
		minute = std::stoi(m[5].str());
	}
	else if (std::regex_match(str, m, regDateTimeNoYear)) {
		month = std::stoi(m[1].str());
		day = std::stoi(m[2].str());
		hour = std::stoi(m[3].str());
		minute = std::stoi(m[4].str());
	}
	else if (std::regex_match(str, m, regTime)) {
		month = baseTime.GetMonth();
		day = baseTime.GetDay();
		hour = std::stoi(m[1].str());
		minute = std::stoi(m[2].str());
	}
	else {
		return false;
	}

	return MakeDateTime(year, month, day, hour, minute, result);
}

} // namespace

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
  時間差の入力を解析する
	@return true:解析成功  false:形式が一致しない、または日時の範囲外
	@param[in]  text   入力文字列
	@param[out] result 時間差(左辺 - 右辺)
*/
bool DateTimeCommandProvider::ParseTimeSpan(const CString& text, CTimeSpan& result)
{
	return ParseTimeSpan(text, result, CTime::GetCurrentTime(), CString());
}

bool DateTimeCommandProvider::ParseTimeSpan(const CString& text, CTimeSpan& result, const CTime& baseTime, const CString& prefix)
{
	CString expression(text);
	expression.Trim();
	// 日付中のハイフンと演算子を区別するため、全てのハイフン位置を候補として解析する
	for (int index = 0; index < expression.GetLength(); ++index) {
		if (expression[index] != _T('-')) {
			continue;
		}

		CString leftText = expression.Left(index);
		CString rightText = expression.Mid(index + 1);
		leftText.Trim();
		rightText.Trim();

		CTime leftTime;
		CTime rightTime;
		if (ParseDateTimeOperand(leftText, baseTime, prefix, leftTime) == false ||
		    ParseDateTimeOperand(rightText, baseTime, prefix, rightTime) == false) {
			continue;
		}

    if (leftTime == rightTime) {
      // 左右が同じ時刻の場合、結果を表示しても有益でなさそうなので省く
      return false;
    }

		result = leftTime - rightTime;
		return true;
	}
	return false;
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

	// 時間差の入力: プレフィックス不要
	CTimeSpan result;
	if (ParseTimeSpan(cmdline, result, CTime::GetCurrentTime(), in->mPrefix)) {
		if (result.GetTotalSeconds() > 24 * 60 * 60 || result.GetTotalSeconds() < -24 * 60 * 60) {
			commands.Add(CommandQueryItem(Pattern::WholeMatch, new DateTimeCommand(result, TYPE_DAY)));
		}
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

