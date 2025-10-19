#include "pch.h"
#include "VKScanner.h"
# define Py_LIMITED_API 0x030c0000
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/list.h>
#include <atlstr.h>
#include <cctype>
#include <vector>

namespace nb = nanobind;

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

int press(nb::handle input, int repeat, float interval)
{
	DWORD interval_ms = (DWORD)(interval * 1000);

	bool isFirst = true;

	for (int i = 0; i < repeat; ++i) {
		if (nb::isinstance<nb::str>(input)) {

			if (isFirst == false) {
				Sleep(interval_ms);
			}
			isFirst = false;

			std::string c = nb::cast<std::string>(input);

			key_down(c.c_str());
			key_up(c.c_str());


		}
		else if (nb::isinstance<nb::list>(input)) {

			for (auto item : nb::cast<nb::list>(input)) {

				if (isFirst == false) {
					Sleep(interval_ms);
				}
				isFirst = false;

				std::string c = nb::cast<std::string>(item);
				key_down(c.c_str());
				key_up(c.c_str());
			}
		}
	}

	return 0;
}

int write_key(nb::handle input, float interval)
{
	DWORD interval_ms = (DWORD)(interval * 1000);

	if (nb::isinstance<nb::str>(input) == false) {
		return 1;
	}

	std::string text = nb::cast<std::string>(input);

	for (auto c : text) {

		bool isUpper = (std::isupper(c) != 0);

		if (isUpper) {
			key_down("shift");
		}

		char text[] = { c, '\0' };
		key_down(text);
		key_up(text);

		if (isUpper) {
			key_up("shift");
		}

		if (interval_ms > 0) {
			Sleep(interval_ms);
		}
	}

	return 0;
}

int hotkey(nb::args args)
{
	std::vector<INPUT> inputs;

	// キーを順に押す
	for (auto arg : args) {

		if (nb::isinstance<nb::str>(arg) == false) {
			continue;
		}

		std::string keystr = nb::cast<std::string>(arg);

		VKScanner scanner(keystr.c_str());

		KEY_DEFINE key;
		while(scanner.Get(&key)) {

			INPUT input;
			if (key.mVK2 != 0) {
				input.type = INPUT_KEYBOARD;
				input.ki.wVk = key.mVK2;
				input.ki.dwFlags = 0;
				inputs.push_back(input);
			}

			input.type = INPUT_KEYBOARD;
			input.ki.wVk = key.mVK;
			input.ki.dwFlags = 0;
			inputs.push_back(input);

			scanner.Next(nullptr);
		}
	}
	// 逆順にキーを放す
	std::vector<INPUT> reverse_inputs;

	for (auto it = inputs.rbegin(); it != inputs.rend(); ++it) {
		auto input = *it;
		input.ki.dwFlags = KEYEVENTF_KEYUP;
		reverse_inputs.push_back(input);
	}

	inputs.insert(inputs.end(), reverse_inputs.begin(), reverse_inputs.end());

	SendInput((UINT)inputs.size(), inputs.data(), sizeof(INPUT));

	return 0;
}


NB_MODULE(key, m) {
	m.def("press", &press, nb::arg("input"), nb::arg("repeat") = 1, nb::arg("interval") = 0.0, "キーを入力する(押して放す)");
	m.def("down", &key_down, "キーを押す");
	m.def("up", &key_up, "キーを放す");
	m.def("write", &write_key, nb::arg("input"), nb::arg("interval") = 0.0, "文字列を入力する");
	m.def("hotkey", &hotkey, "キーを同時入力する");
}
