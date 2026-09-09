#pragma once

#include "MonitorUtility.h"

#include <Windows.h>
#include <physicalmonitorenumerationapi.h>
#include <lowlevelmonitorconfigurationapi.h>

#include <optional>
#include <string>
#include <vector>

struct MonitorDevice
{
	HANDLE handle = nullptr;
	std::wstring displayName;
	bool controllable = false;
};

struct BrightnessInfo
{
	unsigned int current;
	unsigned int maximum;
};

/**
 * 接続された物理モニターを列挙し、ハンドルを管理する
 */
class MonitorSession
{
public:
	MonitorSession() = default;
	~MonitorSession();
	MonitorSession(const MonitorSession&) = delete;
	MonitorSession& operator=(const MonitorSession&) = delete;

	/**
	 * モニターを列挙する
	 *
	 * @param[out] error エラー内容
	 * @return true:成功 false:失敗
	 */
	bool Enumerate(std::string& error);
	const std::vector<MonitorDevice>& GetDevices() const;

private:
	std::vector<MonitorDevice> mDevices;
};

/**
 * モニターから輝度を取得する
 *
 * @param[in] device 対象モニター
 * @param[out] brightness 輝度情報
 * @return true:成功 false:失敗
 */
bool GetMonitorBrightness(const MonitorDevice& device, BrightnessInfo& brightness);

/**
 * モニターの輝度を設定する
 *
 * @param[in] device 対象モニター
 * @param[in] value 正規化された輝度値
 * @return true:成功 false:失敗
 */
bool SetMonitorBrightness(const MonitorDevice& device, int value);

/**
 * モニターがサポートする入力ソースを取得する
 *
 * @param[in] device 対象モニター
 * @param[out] sources 入力ソース一覧
 * @return true:成功 false:失敗
 */
bool GetMonitorInputSources(const MonitorDevice& device, std::vector<InputSourceInfo>& sources);

/**
 * モニターの入力ソースを設定する
 *
 * @param[in] device 対象モニター
 * @param[in] value VCP値
 * @return true:成功 false:失敗
 */
bool SetMonitorInputSource(const MonitorDevice& device, unsigned int value);
