// あ
#pragma once

#include "setting/AppPreferenceListenerIF.h"
#include <memory>

// 入力画面上の基本操作用ホットキーの登録・解除を行うクラス
class MainWindowHotKey : public AppPreferenceListenerIF
{
	class UpHandler;
	class DownHandler;
	class EnterHandler;
	class ComplHandler;
	class ContextMenuHandler;
	class CopyHandler;
public:
	MainWindowHotKey();
	virtual ~MainWindowHotKey();

public:
	// 設定ファイルから設定値を取得してホットキー登録
	bool Register();
	// 登録解除する
	void Unregister();

	// 再登録(登録解除→登録)
	bool Reload();

	CString ToString() const;

private:
	void OnAppFirstBoot() override;
	void OnAppNormalBoot() override;
	void OnAppPreferenceUpdated() override;
	void OnAppExit() override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


