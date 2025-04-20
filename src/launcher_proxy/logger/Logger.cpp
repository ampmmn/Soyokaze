#include "Logger.h"
#include <spdlog/sinks/rotating_file_sink.h>
#include <chrono>
#include <shlwapi.h>

struct Logger::PImpl
{
	PImpl() 
	{
	}
	~PImpl() 
	{
	}
	std::wstring mLogDirectory;
};

Logger::Logger() : in(new PImpl)
{
}

Logger::~Logger()
{
}

Logger* Logger::Get()
{
	static Logger logger;
	return &logger;
}

std::wstring Logger::GetLogDirectory()
{
	return in->mLogDirectory;
}

void Logger::Initialize()
{
	// 通常ログ
	InitializeDefaultLog();
	// 性能計測用ログ
	InitializePerformanceLog();
}

void Logger::InitializeDefaultLog()
{
	wchar_t logPath[256];
	GetModuleFileName(nullptr, logPath, 256);
	PathRemoveFileSpec(logPath);
	in->mLogDirectory = logPath;

	PathAppend(logPath, L"launcher_proxy.log");

	auto max_size{1048576 * 8};
	auto max_files{3};
	auto logger = spdlog::rotating_logger_mt("proxy_logger", (LPCTSTR)logPath, max_size, max_files);
	logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%5t] [%L]:%!:%v");

	int level = spdlog::level::debug;
	spdlog::set_level((spdlog::level::level_enum)level);
	spdlog::flush_every(std::chrono::seconds(3));

	spdlog::set_default_logger(logger);
}


void Logger::InitializePerformanceLog()
{
	wchar_t logPath[256];
	GetModuleFileName(nullptr, logPath, 256);
	PathRemoveFileSpec(logPath);
	PathAppend(logPath, L"launcher_proxy_perf.log");


	auto max_size{1048576 * 8};
	auto max_files{3};
	auto logger = spdlog::rotating_logger_mt("lancuher_perflog", (LPCTSTR)logPath, max_size, max_files);
	logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%5t] [%L]:%!:%v");

	logger->set_level(spdlog::level::debug);
}

