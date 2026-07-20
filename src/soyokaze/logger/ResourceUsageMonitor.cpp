#include "pch.h"
#include "ResourceUsageMonitor.h"
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <format>
#include <chrono>
#include "utility/ScopeExit.h"
#include "utility/Path.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
	UINT TIMERID_INTERNALWINDOW = 1;
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
	}

	// 内部ウインドウ
	HWND mHwnd{nullptr};

	// 記録する間隔
	uint32_t mIntervalInMinutes{20};  // 20分間隔
	// 前回出力した時刻
	uint64_t mLastLoggedTimeStamp{0};
	// 出力を有効にするか?
	bool mIsEnable{false};
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
		return false;
	}

	// 前回の出力から時間が経過していなければ出力しない
	if (GetTickCount64() - in->mLastLoggedTimeStamp < in->mIntervalInMinutes * 1000 * 60) {
		return false;
	}

	std::wstring logPath;
	GetLogFilePath(logPath);

	BOOL isAlreadExists = Path::FileExists(logPath.c_str());

	FILE* fp = nullptr;
	if (_wfopen_s(&fp, logPath.c_str(), L"a") != 0) {
		// 出力できない場合は機能を無効化する
		spdlog::error(L"Failed to create resource usage log. {}",  logPath);
		in->mIsEnable = false;
		return false;
	}

	std::string line;
	if (isAlreadExists == FALSE) {
		// 初回はヘッダ行を出力
		MakeHeader(line);
		fputs(line.c_str(), fp);
	}
	// 1行分の情報を出力
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
}

bool ResourceUsageMonitor::GetWorkingSet(uint64_t* workingSet, uint64_t* workingSetPeak)
{
	PROCESS_MEMORY_COUNTERS_EX pmc = { sizeof(PROCESS_MEMORY_COUNTERS_EX) };

	BOOL isOK = GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc));
	if (isOK == FALSE) {
		return false;
	}

	if (workingSet) {
		*workingSet = pmc.WorkingSetSize;
	}
	if (workingSetPeak) {
		*workingSetPeak = pmc.PeakWorkingSetSize;
	}

	return true;
}

bool ResourceUsageMonitor::GetPrivateBytes(uint64_t* privateBytes)
{
	PROCESS_MEMORY_COUNTERS_EX pmc = { sizeof(PROCESS_MEMORY_COUNTERS_EX) };

	BOOL isOK = GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc));
	if (isOK == FALSE) {
		return false;
	}

	if (privateBytes) {
		*privateBytes = pmc.PrivateUsage;
	}

	return true;
}

bool ResourceUsageMonitor::GetThreadUsage(uint32_t* numOfThreads)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	::utility::ScopeExit guard([&]() { CloseHandle(hSnap); });

	PROCESSENTRY32 pe = { sizeof(pe) };
	if (Process32First(hSnap, &pe) == FALSE) {
		return false;
	}

	auto pid = GetCurrentProcessId();
	do {
		if (pe.th32ProcessID != pid) {
			continue;
		}
		if (numOfThreads) {
			*numOfThreads = pe.cntThreads;
		}
		return true;

	} while (Process32Next(hSnap, &pe));

	return false;
}

bool ResourceUsageMonitor::GetGdiObjects(uint32_t* numOfObjects)
{
	DWORD gdiCount = GetGuiResources( GetCurrentProcess(), GR_GDIOBJECTS);

#ifndef SOYOKAZE_UNITTEST
	if (gdiCount == 0) {
		return false;
	}
#endif

	if (numOfObjects) {
		*numOfObjects = gdiCount;
	}
	return true;
}

bool ResourceUsageMonitor::GetUserObjects(uint32_t* numOfObjects)
{
	DWORD userObjCount = GetGuiResources( GetCurrentProcess(), GR_USEROBJECTS);

#ifndef SOYOKAZE_UNITTEST
	if (userObjCount == 0) {
		return false;
	}
#endif

	if (numOfObjects) {
		*numOfObjects = userObjCount;
	}
	return true;
}

bool ResourceUsageMonitor::MakeLogEntry(std::string& entry)
{
	uint64_t workingSet{0};
	GetWorkingSet(&workingSet, nullptr);
	uint64_t privateBytes{0};
	GetPrivateBytes(&privateBytes);
	uint32_t numOfThreads{0};
	GetThreadUsage(&numOfThreads);
	uint32_t gdiObjects{0};
	GetGdiObjects(&gdiObjects);
	uint32_t userObjects{0};
	GetUserObjects(&userObjects);

	auto zt = std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::system_clock::now()};
	auto timeStr = std::format("{:%Y/%m/%d %H:%M}", zt);

	auto pid = GetCurrentProcessId();
	entry = std::format("{0},{1},{2},{3},{4},{5},{6}\n",
	                    timeStr, GetCurrentProcessId(), workingSet, privateBytes, numOfThreads, gdiObjects, userObjects);

	return true;
}

bool ResourceUsageMonitor::MakeHeader(std::string& header)
{
	header = "Time,PID,WorkingSet,PrivateBytes,Threads,GDI Objects, User Objects\n";
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
