#include "pch.h"
#include "MainWindowDeactivateBlocker.h"
#include "SharedHwnd.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


MainWindowDeactivateBlocker::MainWindowDeactivateBlocker()
{
	// フォーカスを失ったときに隠れるのを阻害する
	SharedHwnd mainWnd;
	SendMessage(mainWnd.GetHwnd(), WM_APP + 14, 0, 1);

}

MainWindowDeactivateBlocker::~MainWindowDeactivateBlocker()
{
	// 状態をもとに戻す
	SharedHwnd mainWnd;
	SendMessage(mainWnd.GetHwnd(), WM_APP + 14, 0, 0);
}
