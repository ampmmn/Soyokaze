#include "pch.h"
#include "ResourceUsageMonitor.h"
#include <windows.h>
#include <format>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <vector>
#include "utility/Path.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
	UINT TIMERID_INTERNALWINDOW = 1;

	// CSVの規則に従い、カンマなどを含む値を必要に応じて引用符で囲む。
	std::string EscapeCsv(const std::string& value)
	{
		if (value.find_first_of(",\"\r\n") == std::string::npos) {
			return value;
		}

		std::string escaped = "\"";
		for (char c : value) {
			escaped += c;
			if (c == '\"') {
				escaped += '\"';
			}
		}
		escaped += "\"";
		return escaped;
	}
}

namespace logger {

struct ResourceUsageMonitor::PImpl : public AppPreferenceListenerIF
{
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override 
	{
		LoadSettings();
	}
	void OnAppExit()
	{
		if (IsWindow(mHwnd)) {
			DestroyWindow(mHwnd);
			mHwnd = nullptr;
		}
	}

	void LoadSettings(){
		auto pref = AppPreference::Get();
		mIsEnable = pref->UseResourceUsageMonitor();
		UNITTESTLOG("mIsEnabel set to true.");
	}

	// 内部ウインドウ
	HWND mHwnd{nullptr};

	// 記録する間隔
	uint32_t mIntervalInMinutes{20};  // 20分間隔
	// 前回出力した時刻
	uint64_t mLastLoggedTimeStamp{0};
	// 出力を有効にするか?
	bool mIsEnable{false};
	std::vector<IResourceMetrics*> mMetrics;
};


ResourceUsageMonitor::ResourceUsageMonitor() : in(new PImpl)
{
}

ResourceUsageMonitor::~ResourceUsageMonitor()
{
}

ResourceUsageMonitor* ResourceUsageMonitor::Get()
{
	static ResourceUsageMonitor inst;
	return &inst;
}

// 初期化
// メインスレッドから呼ぶ想定
bool ResourceUsageMonitor::Initialize()
{
	in->LoadSettings();

	// 内部のmessage処理用の不可視のウインドウを作っておき、タイマーイベントを使って一定間隔でログを出力する
	HINSTANCE hInst = GetModuleHandle(nullptr);
	HWND hwnd = CreateWindowEx(0, _T("STATIC"), _T("LncrResourceUsageMonitor"), 0, 
	                           0, 0, 1, 1,
	                           nullptr, nullptr, hInst, nullptr);

	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)ResourceUsageMonitor::OnWindowProc);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

	// アクティブなウインドウを監視用のタイマーを作っておく
	::SetTimer(hwnd, TIMERID_INTERNALWINDOW, 60 * 1000, 0);

	in->mHwnd = hwnd;
	in->mLastLoggedTimeStamp = GetTickCount64();

	return true;
}

bool ResourceUsageMonitor::Finalize()
{
	in->OnAppExit();
	return true;
}

// ログを出力
bool ResourceUsageMonitor::LogUsage()
{
	if (in->mIsEnable == false) {
		// 機能が無効化されている
		UNITTESTLOG("ResourceUsageMonitor is not enabled.");

		return false;
	}

	// 前回の出力から時間が経過していなければ出力しない。
	if (GetTickCount64() - in->mLastLoggedTimeStamp < in->mIntervalInMinutes * 1000 * 60) {
		UNITTESTLOG("Skip: Not enough time has passed since the last run.");
		return false;
	}

	std::wstring logPath;
	GetLogFilePath(logPath);

	FILE* fp = nullptr;
	std::string header;
	MakeHeader(header);
	bool rewrite = true;
	{
		// 既存ファイルのヘッダーと現在のメトリクス構成を比較する。
		// 構成が変わっている場合は、過去の列構成へ追記しない。
		std::ifstream existing(std::filesystem::path(logPath), std::ios::binary);
		if (existing.is_open()) {
			std::string currentHeader;
			std::getline(existing, currentHeader);
			if (!currentHeader.empty() && currentHeader.back() == '\r') {
				currentHeader.pop_back();
			}
			if (currentHeader == header.substr(0, header.size() - 1)) {
				rewrite = false;
			}
		}
	}

	if (_wfopen_s(&fp, logPath.c_str(), rewrite ? L"wb" : L"ab") != 0) {
		// 出力できない場合は機能を無効化する
		spdlog::error(L"Failed to create resource usage log. {}",  logPath);
		in->mIsEnable = false;
		return false;
	}
	__assume(fp != nullptr);
	if (rewrite) {
		// ファイルがない場合、またはヘッダーが変わった場合は新しいヘッダーを書く。
		fputs(header.c_str(), fp);
	}
	// 登録済みメトリクスの現在値を1行分出力する。
	std::string line;
	MakeLogEntry(line);
	fputs(line.c_str(), fp);

	fclose(fp);

	// 最終出力時刻を更新
	UpdateLastLoggedTimeStamp(GetTickCount64());

	return true;
}

void ResourceUsageMonitor::UpdateLastLoggedTimeStamp(uint64_t n)
{
	in->mLastLoggedTimeStamp = n;
}

// テスト用に強制的に有効にする
void ResourceUsageMonitor::Enable()
{
	in->mIsEnable = true;
	UNITTESTLOG("mIsEnabel set to true.");
}

bool ResourceUsageMonitor::RegisterMetrics(IResourceMetrics* metrics)
{
	if (metrics == nullptr) {
		spdlog::error("Failed to register resource metrics: null metrics");
		return false;
	}
	if (std::any_of(in->mMetrics.begin(), in->mMetrics.end(), [&](auto registered) {
		return registered->GetOrder() == metrics->GetOrder();
	})) {
		// OrderはCSV列順を決めるため、重複した状態では出力構成を確定できない。
		spdlog::error("Failed to register resource metrics: duplicate order {}", metrics->GetOrder());
		return false;
	}
	in->mMetrics.push_back(metrics);
	return true;
}

bool ResourceUsageMonitor::MakeLogEntry(std::string& entry)
{
	auto zt = std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::system_clock::now()};
	auto timeStr = std::format("{:%Y/%m/%d %H:%M}", zt);

	// 登録順に依存せず、Orderの昇順で列を並べる。
	std::vector<IResourceMetrics*> metrics = in->mMetrics;
	std::sort(metrics.begin(), metrics.end(), [](auto left, auto right) {
		return left->GetOrder() < right->GetOrder();
	});
	entry = std::format("{},{}", timeStr, GetCurrentProcessId());
	for (auto metric : metrics) {
		entry += "," + EscapeCsv(metric->GetValue());
	}
	entry += "\n";

	return true;
}

bool ResourceUsageMonitor::MakeHeader(std::string& header)
{
	// ヘッダーもデータ行と同じ順序でメトリクスを並べる。
	std::vector<IResourceMetrics*> metrics = in->mMetrics;
	std::sort(metrics.begin(), metrics.end(), [](auto left, auto right) {
		return left->GetOrder() < right->GetOrder();
	});
	header = "Time,PID";
	for (auto metric : metrics) {
		header += "," + EscapeCsv(metric->GetName());
	}
	header += "\n";
	return true;
}

bool ResourceUsageMonitor::GetLogFilePath(std::wstring& path)
{
	Path logPath(Path::APPDIRPERMACHINE, _T("resource_usage.csv"));
	path = (LPCWSTR)logPath;
	return true;
}

LRESULT ResourceUsageMonitor::OnWindowProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_TIMER) {
		auto thisPtr = (ResourceUsageMonitor*) GetWindowLongPtr(h, GWLP_USERDATA);
		if (thisPtr) {
			thisPtr->LogUsage();
		}
	}
	return DefWindowProc(h, msg, wp, lp);
}

}
