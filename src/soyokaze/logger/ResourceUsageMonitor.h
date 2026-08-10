#pragma once

#include <memory>
#include <string>
#include "IResourceMetrics.h"

namespace logger {

class ResourceUsageMonitor
{
	ResourceUsageMonitor();
	~ResourceUsageMonitor();

public:
	static ResourceUsageMonitor* Get();

	bool Initialize();
	bool Finalize();

	// 登録済みメトリクスからCSVを生成し、必要に応じてファイルへ出力する。
	bool LogUsage(bool isForce = false);
	// メトリクスを登録する。Orderが重複する場合は登録に失敗する。
	bool RegisterMetrics(IResourceMetrics* metrics);

	// テスト用
	void UpdateLastLoggedTimeStamp(uint64_t n);
	void Enable();

	// 固定列と登録済みメトリクスからCSVヘッダーを生成する。
	bool MakeHeader(std::string& header);
	// 固定列と登録済みメトリクスからCSVデータ行を生成する。
	bool MakeLogEntry(std::string& entry);

	static bool GetLogFilePath(std::wstring& path);

private:
	static LRESULT CALLBACK OnWindowProc(HWND h, UINT msg, WPARAM wp, LPARAM lp);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};


}

