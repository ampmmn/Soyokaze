#pragma once

namespace launcherapp {
namespace mainwindow {
namespace layout {

// メインウインドウ上の各部品を所定の位置に配置する
class ComponentPlacer
{
public:
	virtual ~ComponentPlacer(){}

	// アイコン欄のサイズ計算と配置
	virtual bool PlaceIcon(HWND elemHwnd) = 0;
	// 説明欄のサイズ計算と配置
	virtual bool PlaceDescription(HWND elemHwnd) = 0;
	// ガイド欄のサイズ計算と配置
	virtual bool PlaceGuide(HWND elemHwnd) = 0;
	// 入力欄のサイズ計算と配置
	virtual bool PlaceEdit(HWND elemHwnd) = 0;
	// 候補欄のサイズ計算と配置
	virtual bool PlaceCandidateList(HWND elemHwnd) = 0;
	// 適用
	virtual void Apply(HWND mainWindow) = 0;
	// それより以下のリサイズを許容しない最小限の高さを取得する
	virtual int GetMinimumHeight() = 0;
	// 最低限の候補欄の高さを取得する
	virtual int GetMinimumCandidateHeight() = 0;

};

}
}
}


