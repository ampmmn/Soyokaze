#include "pch.h"
#include "framework.h"
#include "utility/AppProfile.h"
#include "utility/Path.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "app/AppName.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <chrono>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct Logger::PImpl : public AppPreferenceListenerIF
{
	PImpl() 
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl() 
	{
	}

	void OnAppFirstBoot() override
	{
	}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		// ログレベルを取得
		auto pref = AppPreference::Get();
		int level = pref->GetLogLevel();
		spdlog::set_level((spdlog::level::level_enum)level);
	}
	void OnAppExit() override
	{
	}
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

void Logger::Initialize()
{
	Path logPath(Path::APPDIR, APPLOGNAME);

	auto max_size = 1048576 * 8;
	auto max_files = 3;
	auto logger = spdlog::rotating_logger_mt("lancuher_logger", (LPCTSTR)logPath, max_size, max_files);
	logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%5t] [%L]:%!:%v");

	auto pref = AppPreference::Get();
	int level = pref->GetLogLevel();
	spdlog::set_level((spdlog::level::level_enum)level);
	spdlog::flush_every(std::chrono::seconds(3));

	spdlog::set_default_logger(logger);
}


