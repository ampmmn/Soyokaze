#pragma once

namespace util {
namespace window {

// ウインドウ位置情報の保存
bool SavePlacement(CWnd* wnd, LPCTSTR key);
// ウインドウ位置情報の復元
bool LoadPlacement(CWnd* wnd, LPCTSTR key);

}  // end of namespace window
}  // end of namespace util

