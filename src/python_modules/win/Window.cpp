#include "pch.h"
# define Py_LIMITED_API 0x030c0000
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/list.h>
#include <atlstr.h>
#include "StringUtil.h"

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
	bool set_position(nanobind::handle insert_after, int x, int y, int width, int height) {
		if (IsWindow(mHwnd) == FALSE) { return false; }

		SetWindowPos(mHwnd, nullptr, x, y, width, height, SWP_NOACTIVATE | SWP_NOOWNERZORDER);
		return true;
	}

	// ウインドウのクライアント領域からの相対座標指定によるクリック
	bool click(nanobind::object x_, nanobind::object y_)
	{
		if (IsWindow(mHwnd) == FALSE) { return false; }
		if (x_.is_none() || y_.is_none()) {
			return false;
		}

		try {
			// クライアント領域をスクリーン領域座標に変換
			POINT pos{ nanobind::cast<int>(x_),nanobind::cast<int>(y_) };
			ClientToScreen(mHwnd, &pos);

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


Window find_window(nanobind::object clsName, nanobind::object caption)
{
	const wchar_t* cls = nullptr;
	std::wstring clsNameTmp;
	if (clsName.is_none() == false) {
		clsNameTmp = utf2utf(nanobind::cast<std::string>(clsName), clsNameTmp);
		cls = clsNameTmp.c_str();
	}

	const wchar_t* cap = nullptr;
	std::wstring captionTmp;
	if (caption.is_none() == false) {
		captionTmp = utf2utf(nanobind::cast<std::string>(caption), captionTmp);
		cap = captionTmp.c_str();
	}

	HWND h = FindWindowW(cls, cap);
	Window w;
	w.mHwnd = h;
	return w;
}

NB_MODULE(win, m) {

	nanobind::class_<Window>(m, "Window")
		.def(nanobind::init<>())
		.def("maximize", &Window::maximize)
		.def("minimize", &Window::minimize)
		.def("show", &Window::show)
		.def("hide", &Window::hide)
		.def("restore", &Window::restore)
		.def("activate", &Window::activate)
		.def("move", &Window::move, nanobind::arg("x"), nanobind::arg("y"))
		.def("resize", &Window::resize, nanobind::arg("width"), nanobind::arg("height"))
		.def("set_position", &Window::set_position, nanobind::arg("insert_after"), nanobind::arg("x"), nanobind::arg("y"), nanobind::arg("width"), nanobind::arg("height"))
		.def("click", &Window::click, nanobind::arg("x"), nanobind::arg("y"));

	m.def("find", &find_window, nanobind::arg("class_name") = nanobind::none(), nanobind::arg("caption") = nanobind::none(), "ウインドウを探す");

}


