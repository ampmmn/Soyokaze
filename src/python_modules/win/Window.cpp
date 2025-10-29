#include "pch.h"
# define Py_LIMITED_API 0x030c0000
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/list.h>
#include <atlstr.h>
#include "StringUtil.h"

namespace nb = nanobind;

class ScopeAttachThreadInput
{
public:
	ScopeAttachThreadInput() : 
		target(GetWindowThreadProcessId(::GetForegroundWindow(),NULL))
 	{
		AttachThreadInput(GetCurrentThreadId(), target, TRUE);
	}

	ScopeAttachThreadInput(DWORD target) : target(target) {
		AttachThreadInput(GetCurrentThreadId(), target, TRUE);
	}

	~ScopeAttachThreadInput()
	{
		AttachThreadInput(GetCurrentThreadId(), target, FALSE);
	}

	DWORD target;
};

class Window
{
public:
	Window() {}

	bool maximize() {
		if (IsWindow(mHwnd) == FALSE) { return false; }
		LONG_PTR style = GetWindowLongPtr(mHwnd, GWL_STYLE);
		if ((style & WS_MAXIMIZE) == 0) {
			PostMessage(mHwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		}
		return true;
	}
	bool minimize() {
		if (IsWindow(mHwnd) == FALSE) { return false; }
		LONG_PTR style = GetWindowLongPtr(mHwnd, GWL_STYLE);
		if ((style & WS_MINIMIZE) == 0) {
			ShowWindow(mHwnd, SW_MINIMIZE);
		}
		return true;
	}
	bool show() {
		if (IsWindow(mHwnd) == FALSE) { return false; }
		ShowWindow(mHwnd, SW_SHOW);
		return true;
	}
	bool hide() {
		if (IsWindow(mHwnd) == FALSE) { return false; }
		ShowWindow(mHwnd, SW_HIDE);
		return true;
	}
	bool restore() {
		if (IsWindow(mHwnd) == FALSE) { return false; }

		ScopeAttachThreadInput scope;
		PostMessage(mHwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		return true;
	}
	bool activate() {
		if (IsWindow(mHwnd) == FALSE) { return false; }

		ScopeAttachThreadInput scope;
		SetForegroundWindow(mHwnd);
		return true;
	}
	bool move(int x, int y) {
		if (IsWindow(mHwnd) == FALSE) { return false; }

		SetWindowPos(mHwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
		return true;
	}
	bool resize(int width, int height) {
		if (IsWindow(mHwnd) == FALSE) { return false; }

		SetWindowPos(mHwnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
		return true;
	}
	bool set_position(nb::handle insert_after, int x, int y, int width, int height) {
		if (IsWindow(mHwnd) == FALSE) { return false; }

		SetWindowPos(mHwnd, nullptr, x, y, width, height, SWP_NOACTIVATE | SWP_NOOWNERZORDER);
		return true;
	}

	bool exists()
	{
		return IsWindow(mHwnd) != FALSE;
	}

	// ウインドウのクライアント領域からの相対座標指定によるクリック
	bool click(nb::object x_, nb::object y_, nb::object label_)
	{
		if (IsWindow(mHwnd) == FALSE) { return false; }

		bool hasLabel = label_.is_none() == false;
		bool hasXY = x_.is_none() == false && y_.is_none() == false;
		if (hasLabel == false && hasXY == false) {
			return false;
		}

		try {
			POINT pos;
			if (hasLabel) {
				// ラベル名から要素を探す
				struct local_param {
					HWND hwnd{nullptr};
					wchar_t buff[1024];
					std::wstring label;
				} param;
				utf2utf(nb::cast<std::string>(label_), param.label);

				// 子ウインドウを列挙してlabelと同じタイトルをもつ要素を探す
				EnumChildWindows(mHwnd, [](HWND h, LPARAM lp) -> BOOL {
					auto param = (local_param*)lp;

					// 子ウインドウのタイトルを取得し、比較
					GetWindowText(h, param->buff, 1024);
					if (param->label != param->buff) {
						// 異なる
						return TRUE;
					}

					// タイトルが一致したのでそれを返す(検索を打ち切る)
					param->hwnd = h;
					return FALSE;
				}, (LPARAM)&param);

				if (IsWindow(param.hwnd) == FALSE) {
					// 見つからなかった
					return false;
				}

				// 子ウインドウの領域を取得
				RECT rcChild;
				GetClientRect(param.hwnd, &rcChild);

				// 中心座標を計算（子ウインドウのクライアント座標系）
				pos.x = (rcChild.right - rcChild.left) / 2;
				pos.y = (rcChild.bottom - rcChild.top) / 2;

				// 子ウインドウのクライアント座標 → スクリーン座標
				ClientToScreen(param.hwnd, &pos);
			}
			else {
				// クライアント領域をスクリーン領域座標に変換
				pos.x = nb::cast<int>(x_);
				pos.y = nb::cast<int>(y_);
				ClientToScreen(mHwnd, &pos);
			}

			// 疑似的なクリック
			return ClickScreen(pos, false);
		}
		catch(std::exception&) {
			return false;
		}
	}

	bool ClickScreen(const POINT& pt, bool isDbl)
	{
		// 現在位置を覚えておく
		POINT curPos;
		if (GetCursorPos(&curPos) == FALSE) {
			return false;
		}

		// カーソル位置を一時的に変更
		SetCursorPos(pt.x, pt.y);

		INPUT inputs[2] = {};

		// マウス左ボタン押下
		inputs[0].type = INPUT_MOUSE;
		inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

		// マウス左ボタン解放
		inputs[1].type = INPUT_MOUSE;
		inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

		// イベント送信
		SendInput(2, inputs, sizeof(INPUT));
		if (isDbl) {
			SendInput(2, inputs, sizeof(INPUT));
		}

		// カーソル位置を戻す
		SetCursorPos(curPos.x, curPos.y);

		return true;
	}

	HWND mHwnd{nullptr};
};


Window find_window(nb::object clsName, nb::object caption, nb::object timeout)
{
	const wchar_t* cls = nullptr;
	std::wstring clsNameTmp;
	if (nb::isinstance<nb::str>(clsName)) {
			clsNameTmp = utf2utf(nb::cast<std::string>(clsName), clsNameTmp);
		cls = clsNameTmp.c_str();
	}

	const wchar_t* cap = nullptr;
	std::wstring captionTmp;
	if (nb::isinstance<nb::str>(caption)) {
		captionTmp = utf2utf(nb::cast<std::string>(caption), captionTmp);
		cap = captionTmp.c_str();
	}

	uint64_t timeout_ms = 0;
	if (nb::isinstance<nb::int_>(timeout) || nb::isinstance<nb::float_>(timeout)) {
		double f = nb::cast<float>(timeout);
		if (f < 0.0) { f = 0.0; }
		timeout_ms = (uint64_t)(f * 1000);
	}

	auto s = GetTickCount64();
	Window w;
	do {
		w.mHwnd = FindWindowW(cls, cap);
		if (w.mHwnd) {
			break;
		}
		Sleep(50);
	}
	while (GetTickCount64() - s < timeout_ms);

	return w;
}

NB_MODULE(win, m) {

	nb::class_<Window>(m, "Window")
		.def(nb::init<>())
		.def("maximize", &Window::maximize)
		.def("minimize", &Window::minimize)
		.def("show", &Window::show)
		.def("hide", &Window::hide)
		.def("restore", &Window::restore)
		.def("activate", &Window::activate)
		.def("move", &Window::move, nb::arg("x"), nb::arg("y"))
		.def("resize", &Window::resize, nb::arg("width"), nb::arg("height"))
		.def("set_position", &Window::set_position, nb::arg("insert_after"), nb::arg("x"), nb::arg("y"), nb::arg("width"), nb::arg("height"))
		.def("click", &Window::click, nb::arg("x") = nb::none(), nb::arg("y") = nb::none(), nb::arg("label") = nb::none())
	  .def_prop_ro("exists", &Window::exists);

	m.def("find", &find_window,
	      nb::arg("class_name") = nb::none(),
	      nb::arg("caption") = nb::none(),
	      nb::arg("timeout") = nb::none(),
	      "ウインドウを探す");

}


