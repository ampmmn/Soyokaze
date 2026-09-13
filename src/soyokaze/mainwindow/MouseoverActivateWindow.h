#pragma once

#include <memory>

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
