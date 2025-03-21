#include "pch.h"
#include "HWNDRect.h"
#include <algorithm>

/**
 * @brief HWNDRect クラスのコンストラクタ
 * @param hwnd 初期化するウィンドウのハンドル
 */
HWNDRect::HWNDRect(HWND hwnd) : mHwnd(hwnd)
{
    // 指定されたウィンドウのクライアント領域の矩形を取得
    GetClientRect(hwnd, &mRect);
}

/**
 * @brief HWNDRect クラスのデストラクタ
 */
HWNDRect::~HWNDRect()
{
}

/**
 * @brief 矩形を別のウィンドウの座標系にマッピングする
 * @param hwnd マッピング先のウィンドウのハンドル
 */
void HWNDRect::MapTo(HWND hwnd)
{
    // クライアント領域の左上と右下の座標を取得
    POINT points[2] = {
        { mRect.left, mRect.top },
        { mRect.right, mRect.bottom },
    };

    // 現在のウィンドウ座標系から指定されたウィンドウ座標系に変換
    MapWindowPoints(mHwnd, hwnd, points, 2);

    // 変換後の座標を使って新しい矩形を作成
    RECT rc;
    rc.left = (std::min)(points[0].x, points[1].x);
    rc.top = (std::min)(points[0].y, points[1].y);
    rc.right = (std::max)(points[0].x, points[1].x);
    rc.bottom = (std::max)(points[0].y, points[1].y);

    // 新しい矩形をメンバ変数に設定し、ウィンドウハンドルを更新
    std::swap(rc, mRect);
    mHwnd = hwnd;
}

/**
 * @brief 矩形を親ウィンドウの座標系にマッピングする
 */
void HWNDRect::MapToParent()
{
    // 親ウィンドウのハンドルを取得し、親ウィンドウの座標系にマッピング
    MapTo(GetParent(mHwnd));
}

/**
 * @brief 矩形の幅を取得する
 * @return 矩形の幅
 */
int HWNDRect::Width() const
{
    return mRect.right - mRect.left;
}

/**
 * @brief 矩形の高さを取得する
 * @return 矩形の高さ
 */
int HWNDRect::Height() const
{
    return mRect.bottom - mRect.top;
}

/**
 * @brief ウィンドウ全体の幅を取得する
 * @return ウィンドウ全体の幅
 */
int HWNDRect::WindowWidth() const
{
    RECT rc;
    // ウィンドウ全体の矩形を取得
    GetWindowRect(mHwnd, &rc);
    return rc.right - rc.left;
}

/**
 * @brief ウィンドウ全体の高さを取得する
 * @return ウィンドウ全体の高さ
 */
int HWNDRect::WindowHeight() const
{
    RECT rc;
    // ウィンドウ全体の矩形を取得
    GetWindowRect(mHwnd, &rc);
    return rc.bottom - rc.top;
}
