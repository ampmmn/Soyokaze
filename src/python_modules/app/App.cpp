#include "pch.h"
# define Py_LIMITED_API 0x030c0000
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/list.h>
#include <atlstr.h>
#include "StringUtil.h"
#include "SharedHwnd.h"

namespace nb = nanobind;


int popup_message(nb::object message)
{
	if (nb::isinstance<nb::str>(message) == false) {
		return 1;
	}

	std::string msg = nb::cast<std::string>(message);

	SharedHwnd mainWindow;
	SendMessage(mainWindow.GetHwnd(), WM_APP+21, 0, (LPARAM)msg.c_str());

	return 0;
}

int run_command(nb::object input_text)
{
	if (nb::isinstance<nb::str>(input_text) == false) {
		return 1;
	}

	std::wstring inputw;
	std::string input = nb::cast<std::string>(input_text);
	utf2utf(input, inputw);

	SharedHwnd mainWindow;
	SendMessage(mainWindow.GetHwnd(), WM_APP+3, 1, (LPARAM)inputw.c_str());

	return 0;
}

std::string expand_macro(nb::object input_text)
{
	if (nb::isinstance<nb::str>(input_text) == false) {
		return "";
	}

	std::wstring inputw;
	std::string input = nb::cast<std::string>(input_text);
	utf2utf(input, inputw);

	wchar_t* expand_result = nullptr;

	SharedHwnd mainWindow;
	SendMessage(mainWindow.GetHwnd(), WM_APP+22, (WPARAM)inputw.c_str(), (LPARAM)&expand_result);

	if (expand_result == nullptr) {
		return "";
	}

	std::string output;
	utf2utf(expand_result, output);

	SendMessage(mainWindow.GetHwnd(), WM_APP+23, 0, (LPARAM)expand_result);

	return output;
}

NB_MODULE(app, m) {

	m.def("popup_message", &popup_message, "メッセージを表示する");
	m.def("run_command", &run_command, "コマンドを実行する");
	m.def("expand_macro", &expand_macro, "マクロを展開する");

}


