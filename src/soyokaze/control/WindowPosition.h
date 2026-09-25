#pragma once

#include <memory>
#include <stdint.h>
#include <vector>

class Path;

class WindowPosition
{
public:
	/** モニター構成変更後の位置復元結果 */
	enum class MonitorChangeResult
	{
		Unchanged,          /**< モニター構成に変化なし */
		PlacementRestored,  /**< 構成に対応する位置情報を復元 */
		NoSavedPlacement    /**< 保存位置がなく現在位置を維持 */
	};

	WindowPosition();
	WindowPosition(LPCTSTR name);
	~WindowPosition();

public:
	bool Restore(HWND hwnd);
	/** モニター構成変更後に位置を復元し、構成変化と位置復元の結果を返す
	  @return 構成の変化と位置復元の結果
	  @param[in] hwnd 対象ウインドウハンドル
	*/
	MonitorChangeResult RestoreForMonitorChange(HWND hwnd);
	bool Update(HWND hwnd);
	bool Save();

	WINDOWPLACEMENT GetPosition() const;

	/**
	  モニター矩形一覧から構成識別子を生成する
	  @return SHA-1フル値を表す16進数文字列
	  @param[in] monitors モニター矩形一覧
	*/
	static CString CreateMonitorConfigurationIdentifier(const std::vector<RECT>& monitors);

	/**
	  バイト列が有効なWINDOWPLACEMENTかを検証する
	  @return true:有効 false:無効
	  @param[in] data 検証するバイト列
	  @param[out] placement 復元したWINDOWPLACEMENT
	*/
	static bool IsValidWindowPlacementData(const std::vector<uint8_t>& data, WINDOWPLACEMENT& placement);

protected:
	static void GetFilePath(LPCTSTR baseName, Path& path);
	/** 現在のモニター構成が最後に復元した構成と一致するか確認する */
	bool IsCurrentMonitorConfiguration() const;
	/**
	  保持しているウインドウ位置を更新する
	  @param[in] position 設定するウインドウ位置
	*/
	void SetPosition(const WINDOWPLACEMENT& position);
	/**
	  位置情報が復元または更新済みか確認する
	  @return true:復元または更新済み false:未設定
	*/
	bool IsPositionLoaded() const;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

