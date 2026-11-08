#pragma once

#define _CRT_SECURE_NO_WARNINGS

#ifdef _UNICODE
#define SPDLOG_WCHAR_FILENAMES
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#endif

#pragma warning( push )
#pragma warning( disable : 26495 26437 26450 26498 26800 6285 6385)
#include "spdlog/spdlog.h"
#include "spdlog/stopwatch.h"
#pragma warning( pop )

class Logger
{
	Logger();
	~Logger();

public:
	static Logger* Get();

	std::wstring GetLogDirectory();

	void Initialize();

private:
	void InitializeDefaultLog();
	void InitializePerformanceLog();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

#define PERFLOG(fmt, ...) spdlog::get("lancuher_perflog")->info(fmt, __VA_ARGS__)

