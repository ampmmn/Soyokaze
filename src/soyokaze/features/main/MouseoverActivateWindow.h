#pragma once

#include <memory>

/**
  カーソル位置によるアクティブ切替の状態を管理する
*/
class MouseoverActivateState
{
public:
	enum class Action
	{
		None,
		Activate,
		Deactivate,
	};

	/**
	  カーソル位置とマウスボタン状態に基づき、実行する切替処理を返す
	  @return 実行する切替処理
	  @param[in]  isInside ウインドウ内にカーソルがあるか
	  @param[in]  isLeftButtonDown マウス左ボタンが押されているか
	*/
	Action Update(bool isInside, bool isLeftButtonDown);

	/**
	  監視状態を初期化する
	*/
	void Reset();

private:
	bool mHasEntered{false};
	bool mWasInside{false};
	bool mWasLeftButtonDown{false};
};


class MouseoverActivateWindow : public CWnd
{
public:
	MouseoverActivateWindow();
	~MouseoverActivateWindow();

	BOOL Create(CWnd* parentWnd);

	/**
	  マウスカーソル位置の監視を一時停止する
	*/
	void Suspend();

	/**
	  マウスカーソル位置の監視を再開する
	*/
	void Resume();

	/**
	  マウスカーソル位置の監視状態を初期化する
	*/
	void ResetState();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

	afx_msg void OnTimer(UINT_PTR timerId);

	DECLARE_MESSAGE_MAP()
};
