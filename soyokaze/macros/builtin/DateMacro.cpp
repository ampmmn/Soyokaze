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

	static tregex pat(_T("([+-])(\\d+)([YMDhms])"));
	if (std::regex_search(str, pat) == false) {
		return CTimeSpan();
	}

	tstring sign = std::regex_replace(str, pat, _T("$1"));
	tstring offset = std::regex_replace(str, pat, _T("$2"));
	tstring unit = std::regex_replace(str, pat, _T("$3"));

	// 符号がマイナスなら反転
	int n = _ttoi(offset.c_str());
	if (sign == _T("-")) {
		n = -n;
	}

	try {
		// 年と月の場合は現在の日付から日数を計算してCTimeSpanを生成
		if (unit == _T("Y")) {
			return CTimeSpan(ComputeDayFromYear(n, CTime::GetCurrentTime()), 0, 0, 0);
		}
		else if (unit == _T("M")) {
			return CTimeSpan(ComputeDayFromMonth(n, CTime::GetCurrentTime()), 0, 0, 0);
		}
		else if (unit == _T("D")) {
			return CTimeSpan(n, 0, 0, 0);
		}
		else if (unit == _T("h")) {
			return CTimeSpan(0, n, 0, 0);
		}
		else if (unit == _T("m")) {
			return CTimeSpan(0, 0, n, 0);
		}
		else /*if (unit == _T("s"))*/ {
			return CTimeSpan(0, 0, 0, n);
		}
	}
	catch(...) {
		SPDLOG_ERROR(_T("Failed to get timespan. {}"), (LPCTSTR)arg);
		return CTimeSpan();
	}
}


}
}
}
