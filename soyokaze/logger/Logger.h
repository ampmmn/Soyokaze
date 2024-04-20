#pragma once

#define _CRT_SECURE_NO_WARNINGS

#ifdef _UNICODE
#define SPDLOG_WCHAR_FILENAMES
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#endif

//#ifdef _DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
//#else
//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_OFF
//#endif

#include "spdlog/spdlog.h"

class Logger
{
	Logger();
	~Logger();

public:
	static Logger* Get();

	void Initialize();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


