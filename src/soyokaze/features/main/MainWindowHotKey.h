// あ
#pragma once

#include "setting/AppPreferenceListenerIF.h"
#include <memory>

// 入力画面上の基本操作用ホットキーの登録・解除を行うクラス
class MainWindowHotKey : public AppPreferenceListenerIF
{
	class HandlerBase;
	class SelectUpHandler;
	class SelectDownHandler;
	class EnterHandler;
	class ComplHandler;
	class DeleteWordHandler;
	class ContextMenuHandler;
	class CopyHandler;
	class MoveUpHandler;
	class MoveDownHandler;
	class MoveLeftHandler;
	class MoveRightHandler;
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


