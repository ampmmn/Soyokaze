#include "pch.h"
#include "CurrencyConvesionProvider.h"
#include "CurrencyConvesionCommand.h"
#include "commands/currency/CurrencyCommandParam.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "utility/CharConverter.h"
#include "utility/Path.h"
#include "utility/WinHttp.h"
#include "utility/Regex.h"
#include <fstream>
#include <map>
#include <atomic>
#include <mutex>
#include <thread>
#include <nlohmann/json.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace currency {

using json = nlohmann::json;
using RateMap = std::map<std::string, double>;

namespace {

// 基準日から指定日数前の日付を得る
CString GetReferenceDate(int daysAgo)
{
	CTime referenceTime = CTime::GetCurrentTime() - CTimeSpan(daysAgo, 0, 0, 0);
	return referenceTime.Format(_T("%Y-%m-%d"));
}

CString GetCachePath()
{
	Path tmpPath(Path::APPDIR, _T("tmp"));
	if (tmpPath.IsDirectory() == false) {
		CreateDirectory(tmpPath, nullptr);
	}

	Path currencyPath((LPCTSTR)tmpPath);
	currencyPath.Append(_T("currencies"));
	if (currencyPath.IsDirectory() == false) {
		CreateDirectory(currencyPath, nullptr);
	}

	currencyPath.Append(_T("eur.json"));
	return (LPCTSTR)currencyPath;
}

bool ParseRates(const std::string& content, const CString& expectedDate, RateMap& rates)
{
	try {
		json data = json::parse(content);
		std::string expectedDateUtf8;
		launcherapp::utility::CharConverter::UTF2UTF(std::wstring(expectedDate), expectedDateUtf8);
		if (data.value("date", "") != expectedDateUtf8 || data.contains("eur") == false || data["eur"].is_object() == false) {
			return false;
		}

		for (auto& item : data["eur"].items()) {
			if (item.value().is_number()) {
				rates[item.key()] = item.value().get<double>();
			}
		}
		return rates.empty() == false;
	}
	catch (const json::exception& e) {
		spdlog::warn("Failed to parse currency data: {}", e.what());
		return false;
	}
}

bool LoadRatesFromCache(const CString& cachePath, const CString& expectedDate, RateMap& rates)
{
	std::ifstream file((LPCTSTR)cachePath, std::ios::binary);
	if (file.is_open() == false) {
		return false;
	}

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return ParseRates(content, expectedDate, rates);
}

bool DownloadRates(const CString& expectedDate, const CString& cachePath, RateMap& rates)
{
	CString url;
	url.Format(_T("https://cdn.jsdelivr.net/npm/@fawazahmed0/currency-api@%s/v1/currencies/eur.json"), (LPCTSTR)expectedDate);

	std::vector<BYTE> content;
	WinHttp http;
	if (http.LoadBinaryContent(url, content) == false) {
		spdlog::warn("Failed to download currency data.");
		return false;
	}

	std::string data(content.begin(), content.end());
	if (ParseRates(data, expectedDate, rates) == false) {
		return false;
	}

	std::ofstream file((LPCTSTR)cachePath, std::ios::binary | std::ios::trunc);
	if (file.is_open()) {
		file.write(data.data(), static_cast<std::streamsize>(data.size()));
	}
	return true;
}

bool LoadRatesWithFallback(const CString& cachePath, RateMap& rates)
{
	for (int daysAgo = 0; daysAgo <= 1; daysAgo++) {
		CString referenceDate = GetReferenceDate(daysAgo);
		RateMap candidateRates;
		if (LoadRatesFromCache(cachePath, referenceDate, candidateRates) ||
			DownloadRates(referenceDate, cachePath, candidateRates)) {
			rates.swap(candidateRates);
			return true;
		}
	}

	return false;
}

std::string ToRateKey(const tstring& currency)
{
	CStringA value(CString(currency.c_str()));
	return std::string((LPCSTR)value);
}

tstring NormalizeCurrency(tstring currency)
{
	std::transform(currency.begin(), currency.end(), currency.begin(), [](TCHAR c) { return (TCHAR)_totlower(c); });
	if (currency == _T("yen")) {
		return _T("jpy");
	}
	return currency;
}

}

struct CurrencyConvesionProvider::PImpl :
	public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this, _T("Curreny"));
	}
	virtual ~PImpl()
	{
		if (mDownloadThread.joinable()) {
			mDownloadThread.join();
		}
		AppPreference::Get()->UnregisterListener(this);
	}

	void Load()
	{
		auto pref = AppPreference::Get();
		mParam.Load((Settings&)pref->GetSettings());
		mIsReady = false;
		{
			std::lock_guard<std::mutex> lock(mMutex);
			mRates.clear();
		}
		if (mParam.mIsEnable == false) {
			return;
		}

		CString cachePath = GetCachePath();

		if (mDownloadThread.joinable() && mIsDownloading == false) {
			mDownloadThread.join();
		}
		if (mDownloadThread.joinable()) {
			return;
		}

		mIsDownloading = true;
		auto th = std::thread([this, cachePath]() {
			RateMap rates;
			bool isReady = LoadRatesWithFallback(cachePath, rates);
			if (isReady) {
				std::lock_guard<std::mutex> lock(mMutex);
				mRates.swap(rates);
			}
			mIsReady = isReady;
			mIsDownloading = false;
		});
		mDownloadThread.swap(th);
	}


// AppPreferenceListenerIF
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Load();
	}
	void OnAppExit() override {}

	CommandParam mParam;
	RateMap mRates;
	std::mutex mMutex;
	std::thread mDownloadThread;
	std::atomic<bool> mIsDownloading{false};
	std::atomic<bool> mIsReady{false};
};

REGISTER_COMMANDPROVIDER(CurrencyConvesionProvider)

CurrencyConvesionProvider::CurrencyConvesionProvider() : in(std::make_unique<PImpl>())
{
}

CurrencyConvesionProvider::~CurrencyConvesionProvider()
{
}

CString CurrencyConvesionProvider::GetName()
{
	return _T("CurrencyConvesion");
}

void CurrencyConvesionProvider::PrepareAdhocCommands()
{
	in->Load();
}

void CurrencyConvesionProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands)
{
	if (in->mParam.mIsEnable == false || in->mIsReady == false) {
		return;
	}

	static const tregex regex(_T("^ *(([-+]?(?:[0-9]+(\\.[0-9]+)?|\\.[0-9]+))) +([A-Za-z]+) +in +([A-Za-z]+) *$"));
	tstring input = (tstring)pattern->GetWholeString();
	std::match_results<tstring::const_iterator> match;
	if (std::regex_match(input, match, regex) == false) {
		return;
	}

	double value = 0.0;
	if (_stscanf_s(match[2].str().c_str(), _T("%lf"), &value) != 1) {
		return;
	}

	tstring source = match[4].str();
	tstring target = match[5].str();
	source = NormalizeCurrency(source);
	target = NormalizeCurrency(target);

	double sourceRate = source == _T("eur") ? 1.0 : 0.0;
	double targetRate = target == _T("eur") ? 1.0 : 0.0;
	std::lock_guard<std::mutex> lock(in->mMutex);
	if (sourceRate == 0.0) {
		auto it = in->mRates.find(ToRateKey(source));
		if (it == in->mRates.end()) {
			return;
		}
		sourceRate = it->second;
		if (sourceRate <= 0.0) {
			return;
		}
	}
	if (targetRate == 0.0) {
		auto it = in->mRates.find(ToRateKey(target));
		if (it == in->mRates.end()) {
			return;
		}
		targetRate = it->second;
		if (targetRate <= 0.0) {
			return;
		}
	}

	double result = value * targetRate / sourceRate;
	commands.Add(CommandQueryItem(Pattern::WholeMatch, new CurrencyConvesionCommand(result, CString(target.c_str()))));
}

uint32_t CurrencyConvesionProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(CurrencyConvesionCommand::TypeDisplayName());
	return 1;
}

}}}
