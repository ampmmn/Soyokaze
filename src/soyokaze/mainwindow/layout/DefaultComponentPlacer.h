#pragma once

#include "mainwindow/layout/ComponentPlacer.h"

#include <memory>

namespace launcherapp {
namespace mainwindow {
namespace layout {

class MainWindowPlacement;

// メインウインドウ上の各部品を所定の位置に配置する
// アプリ初回起動時の設定に基づく配置処理
class DefaultComponentPlacer : public ComponentPlacer
{
public:
	DefaultComponentPlacer(MainWindowPlacement* placement);
	~DefaultComponentPlacer() override;

	// アイコン欄のサイズ計算と配置
	bool PlaceIcon(HWND elemHwnd, LauncherInput* status) override;
	// 説明欄のサイズ計算と配置
	bool PlaceDescription(HWND elemHwnd, LauncherInput* status) override;
	// ガイド欄のサイズ計算と配置
	bool PlaceGuide(HWND elemHwnd, LauncherInput* status) override;
	// 入力欄のサイズ計算と配置
	bool PlaceEdit(HWND elemHwnd, LauncherInput* status) override;
	// 候補欄のサイズ計算と配置
	bool PlaceCandidateList(HWND elemHwnd, LauncherInput* status) override;
	// オプションボタンのサイズ計算と配置
	bool PlaceOptionButton(HWND elemHwnd, LauncherInput* status) override;
	// 適用
	void Apply(HWND hwnd, LauncherInput* status) override;
	// それより以下のリサイズを許容しない最小限の高さを取得する
	int GetMinimumHeight() override;
	// 最低限の候補欄の高さを取得する
	int GetMinimumCandidateHeight() override;

protected:
	void GetGuideRect(CRect& rc);
	void GetEditRect(CRect& rc);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}


