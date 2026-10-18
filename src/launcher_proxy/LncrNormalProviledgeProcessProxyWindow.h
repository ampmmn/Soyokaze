#pragma once

#include <memory>
#include <windows.h>

class LncrNormalProviledgeProcessProxyWindow
{
public:
	/**
	  コンストラクタ
	*/
	LncrNormalProviledgeProcessProxyWindow();
	/**
	  デストラクタ
	*/
	~LncrNormalProviledgeProcessProxyWindow();

	/**
	  ウィンドウ専用スレッドを作成してウィンドウを起動する
	  @param[in] hInst インスタンスハンドル
	  @return true:成功 false:失敗
	*/
	bool Create(HINSTANCE hInst);

	/**
	  リクエスト処理中かどうかを設定する
	  @param[in] isBusy true:処理中 false:受付可能
	*/
	void SetBusy(bool isBusy);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
