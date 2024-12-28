#pragma once

// 一時的にアプリケーションメインウインドウのWS_EX_TOPMOSTを無効化する
class TopMostMask
{
public:
	TopMostMask();
	~TopMostMask();

protected:
	bool mIsMasked;
	HWND mWindow;
};

