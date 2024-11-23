#include "pch.h"
#include "DateMacro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace macros {
namespace builtin {

REGISTER_LAUNCHERMACRO(DateMacro)

DateMacro::DateMacro()
{
	mName = _T("date");
}

DateMacro::~DateMacro()
{
}

int DateMacro::ComputeDayFromYear(int offset, CTime tmToday)
{
	UNREFERENCED_PARAMETER(tmToday);

	// FIXME: うるう年
	return offset * 365;
}

int DateMacro::ComputeDayFromMonth(int offset, CTime tmToday)
{
	static int DAY_TABLE[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	// 日数の合計
	int days = 0;

	// 12より大きいオフセットの場合は年単位で計算する
	int years = offset / 12;

	// 現在の月を得る
	int curMonth = tmToday.GetMonth();

	// 年数分を差し引いた残りの月数を得る
	int count = offset - (years * 12);
	while (count != 0) {

		int m = curMonth;

		while(m <= 0) { m += 12; }
		while(m >= 13) { m -= 12; }

		ASSERT(1 <= m && m <= 12);

		// 月ごとの日数を計算
		days += DAY_TABLE[m-1];

		count += (offset > 0? -1 : 1);
		curMonth += (offset > 0 ? -1 : 1);
	}

	return days + ComputeDayFromYear(years, tmToday);
}

bool DateMacro::Evaluate(const std::vector<CString>& args, CString& result)
{
	CTime tmCur(CTime::GetCurrentTime());

	if (args.size() == 0) {
		// YYYY-mm-dd HH:MM:ss
		result = tmCur.Format(_T("%X"));
		return true;
	}

	if (args.size() >= 2) {
		// オフセット
		tmCur += MakeTimeSpan(args[1]);
	}

	if (args.size() >= 1) {
		// フォーマット
		try {
			auto fmt = args[0];
			result = tmCur.Format(fmt);
			return true;
		}
		catch (...) {
			SPDLOG_ERROR("An exception occurred.");
			return false;
		}
	}
	return false;
}

CTimeSpan DateMacro::MakeTimeSpan(const CString& arg)
{
	tstring str = (LPCTSTR)arg;

	CTime curTime = CTime::GetCurrentTime();

	static tregex pat(_T("^(S)?([+-])(\\d+)([YMDhms])"));
	if (std::regex_match(str, pat) == false) {
		return CTimeSpan();
	}

	tstring head = std::regex_replace(str, pat, _T("$1"));
	tstring sign = std::regex_replace(str, pat, _T("$2"));
	tstring offset = std::regex_replace(str, pat, _T("$3"));
	tstring unit = std::regex_replace(str, pat, _T("$4"));

	// 符号がマイナスなら反転
	int n = _ttoi(offset.c_str());
	if (sign == _T("-")) {
		n = -n;
	}

	try {
		CTimeSpan tmOffset;
		// "S"が指定された場合、その週の日曜日を基準とする
		if (head.empty() == false) {
			tmOffset = CTimeSpan(-(curTime.GetDayOfWeek() - 1), 0, 0, 0);
		}

		// 年と月の場合は現在の日付から日数を計算してCTimeSpanを生成
		if (unit == _T("Y")) {
			tmOffset += CTimeSpan(ComputeDayFromYear(n, curTime), 0, 0, 0);
		}
		else if (unit == _T("M")) {
			tmOffset += CTimeSpan(ComputeDayFromMonth(n, curTime), 0, 0, 0);
		}
		else if (unit == _T("D")) {
			tmOffset += CTimeSpan(n, 0, 0, 0);
		}
		else if (unit == _T("h")) {
			tmOffset += CTimeSpan(0, n, 0, 0);
		}
		else if (unit == _T("m")) {
			tmOffset += CTimeSpan(0, 0, n, 0);
		}
		else /*if (unit == _T("s"))*/ {
			tmOffset += CTimeSpan(0, 0, 0, n);
		}
		return tmOffset;
	}
	catch(...) {
		SPDLOG_ERROR(_T("Failed to get timespan. {}"), (LPCTSTR)arg);
		return CTimeSpan();
	}
}


}
}
}
