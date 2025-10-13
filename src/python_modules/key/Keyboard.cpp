#include "pch.h"
#include "VKScanner.h"
# define Py_LIMITED_API 0x03100000
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/list.h>
#include <atlstr.h>

int key_down(const char* chars)
{
	VKScanner scanner(chars);

	KEY_DEFINE key;
	while(scanner.Get(&key)) {

		INPUT input[2] = {};
		if (key.mVK2 != 0) {
			input[0].type = INPUT_KEYBOARD;
			input[0].ki.wVk = key.mVK2;
			input[0].ki.dwFlags = 0;
		}

		int n = key.mVK2== 0? 0 : 1;
		input[n].type = INPUT_KEYBOARD;
		input[n].ki.wVk = key.mVK;
		input[n].ki.dwFlags = 0;

		SendInput(n+1, input, sizeof(INPUT));

		scanner.Next(nullptr);
	}


	return 0;
}

int key_up(const char* chars)
{
	VKScanner scanner(chars);

	KEY_DEFINE key;
	while(scanner.Get(&key)) {

		INPUT input[2] = {};
		if (key.mVK2 != 0) {
			input[0].type = INPUT_KEYBOARD;
			input[0].ki.wVk = key.mVK2;
			input[0].ki.dwFlags = KEYEVENTF_KEYUP;
		}

		int n = key.mVK2== 0? 0 : 1;
		input[n].type = INPUT_KEYBOARD;
		input[n].ki.wVk = key.mVK;
		input[n].ki.dwFlags = KEYEVENTF_KEYUP;

		SendInput(n+1, input, sizeof(INPUT));

		scanner.Next(nullptr);
	}

	return 0;
}

int press(nanobind::handle input, int repeat, float interval)
{
	DWORD interval_ms = (DWORD)(interval * 1000);

	bool isFirst = true;

	for (int i = 0; i < repeat; ++i) {
		if (nanobind::isinstance<nanobind::str>(input)) {

			if (isFirst == false) {
				Sleep(interval_ms);
			}
			isFirst = false;

			std::string c = nanobind::cast<std::string>(input);

			key_down(c.c_str());
			key_up(c.c_str());


		}
		else if (nanobind::isinstance<nanobind::list>(input)) {

			for (auto item : nanobind::cast<nanobind::list>(input)) {

				if (isFirst == false) {
					Sleep(interval_ms);
				}
				isFirst = false;

				std::string c = nanobind::cast<std::string>(item);
				key_down(c.c_str());
				key_up(c.c_str());
			}
		}
	}

	return 0;
}

NB_MODULE(key, m) {
	m.def("press", &press, nanobind::arg("input"), nanobind::arg("repeat") = 1, nanobind::arg("interval") = 0.0, "キーを押す");
	m.def("key_down", &key_down, "キーを押す");
	m.def("key_up", &key_up, "キーを放す");
}
