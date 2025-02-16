#pragma once

#include "mainwindow/layout/DefaultComponentPlacer.h"

#include <memory>

namespace launcherapp {
namespace mainwindow {
namespace layout {

class MainWindowPlacement;

// メインウインドウ上の各部品を所定の位置に配置する
// アイコン欄表示なし配置処理
class NoIconComponentPlacer : public ComponentPlacer
{
public:
	NoIconComponentPlacer(MainWindowPlacement* placement);
	~NoIconComponentPlacer() override;

	// アイコン欄のサイズ計算と配置
	bool PlaceIcon(HWND elemHwnd) override;
	// 説明欄のサイズ計算と配置
	bool PlaceDescription(HWND elemHwnd) override;
	// ガイド欄のサイズ計算と配置
	bool PlaceGuide(HWND elemHwnd) override;
	// 入力欄のサイズ計算と配置
	bool PlaceEdit(HWND elemHwnd) override;
	// 候補欄のサイズ計算と配置
	bool PlaceCandidateList(HWND elemHwnd) override;
	// 適用
	void Apply(HWND hwnd) override;
	// それより以下のリサイズを許容しない最小限の高さを取得する
	int GetMinimumHeight() override;
	// 最低限の候補欄の高さを取得する
	int GetMinimumCandidateHeight() override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}


