#include "pch.h"
#include "FileTime.h"
#include <chrono>
#include <filesystem>

namespace launcherapp { namespace commands { namespace common {

namespace fs = std::filesystem;

// ファイルの最終更新時刻を取得
bool GetLastUpdateTime(LPCTSTR path, time_t& ftime)
{
	try {
		fs::path filePath(path);
		auto last_write_time = fs::last_write_time(filePath);

		// time_point を time_t に変換
		auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
			last_write_time - fs::file_time_type::clock::now() + std::chrono::system_clock::now());

		ftime = std::chrono::system_clock::to_time_t(sctp);
		return true;
	}
	catch(...) {
		return false;
	}
}


}}} // end of namespace launcherapp::commands::common

