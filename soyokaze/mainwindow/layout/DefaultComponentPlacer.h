#pragma once

#include "mainwindow/layout/ComponentPlacer.h"

#include <memory>

namespace launcherapp {
namespace mainwindow {
namespace layout {

// メインウインドウ上の各部品を所定の位置に配置する
// アプリ初回起動時の設定に基づく配置処理
class DefaultComponentPlacer : public ComponentPlacer
{
public:
	DefaultComponentPlacer(HWND hwnd);
	~DefaultComponentPlacer() override;

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

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}


