#pragma once

// メインウインドウがフォーカスを失ったときに隠れるのを一時的に阻害するためのクラス
class MainWindowDeactivateBlocker
{
public:
	MainWindowDeactivateBlocker();
	~MainWindowDeactivateBlocker();
};
