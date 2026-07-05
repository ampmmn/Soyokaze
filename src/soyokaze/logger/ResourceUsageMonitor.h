#pragma once

#include <memory>
#include <string>

namespace logger {

class ResourceUsageMonitor
{
	ResourceUsageMonitor();
	~ResourceUsageMonitor();

public:
	static ResourceUsageMonitor* Get();

	bool Initialize();
	bool Finalize();

	bool LogUsage();

	// テスト用
	void UpdateLastLoggedTimeStamp(uint64_t n);
	void Enable();

	static bool GetWorkingSet(uint64_t* workingSet, uint64_t* workingSetPeak);
	static bool GetPrivateBytes(uint64_t* privateBytes);
	static bool GetThreadUsage(uint32_t* numOfThreads);
	static bool GetGdiObjects(uint32_t* numOfObjects);
	static bool GetUserObjects(uint32_t* numOfObjects);
	static bool MakeHeader(std::string& header);
	static bool MakeLogEntry(std::string& entry);

	static bool GetLogFilePath(std::wstring& path);

private:
	static LRESULT CALLBACK OnWindowProc(HWND h, UINT msg, WPARAM wp, LPARAM lp);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};


}

