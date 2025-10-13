#include "pch.h"
#include "VKScanner.h"
#include <string>
#include <algorithm>

static KEY_DEFINE keyDefines[] = {
	{ "~", 0xde, VK_SHIFT },
	{ "}", 0xdd, VK_SHIFT },
	{ "|", 0xdc, VK_SHIFT },
	{ "{", 0xdb, VK_SHIFT },
	{ "z", 0x5a, 0 },
	{ "y", 0x59, 0 },
	{ "x", 0x58, 0 },
	{ "win", 0x5b, 0 },
	{ "w", 0x57, 0 },
	{ "v", 0x56, 0 },
	{ "up", 0x26, 0 },
	{ "u", 0x55, 0 },
	{ "tab", 0x09, 0 },
	{ "t", 0x54, 0 },
	{ "shift", 0x10, 0 },
	{ "scrolllock", 0x91, 0 },
	{ "s", 0x53, 0 },
	{ "right", 0x27, 0 },
	{ "return", VK_RETURN, 0 },
	{ "r", 0x52, 0 },
	{ "q", 0x51, 0 },
	{ "printscreen", 0x2c, 0 },
	{ "pause", 0x13, 0 },
	{ "pageup", 0x21, 0 },
	{ "pagedown", 0x22, 0 },
	{ "p", 0x50, 0 },
	{ "o", 0x4f, 0 },
	{ "numlock", 0x90, 0 },
	{ "num9", 0x69, 0 },
	{ "num8", 0x68, 0 },
	{ "num7", 0x67, 0 },
	{ "num6", 0x66, 0 },
	{ "num5", 0x65, 0 },
	{ "num4", 0x64, 0 },
	{ "num3", 0x63, 0 },
	{ "num2", 0x62, 0 },
	{ "num1", 0x61, 0 },
	{ "num0", 0x60, 0 },
	{ "nonconvert", 0x1d, 0 },
	{ "n", 0x4e, 0 },
	{ "m", 0x4d, 0 },
	{ "left", 0x25, 0  },
	{ "l", 0x4c, 0 },
	{ "kanji", 0xf2, 0 },
	{ "kana", 0xf2, 0 },
	{ "k", 0x4b, 0 },
	{ "j", 0x4a, 0 },
	{ "insert", 0x2d, 0 },
	{ "i", 0x49, 0 },
	{ "home", 0x24, 0 },
	{ "h", 0x48, 0 },
	{ "g", 0x47, 0 },
	{ "f9", 0x78, 0 },
	{ "f8", 0x77, 0 },
	{ "f7", 0x76, 0 },
	{ "f6", 0x75, 0 },
	{ "f5", 0x74, 0 },
	{ "f4", 0x73, 0 },
	{ "f3", 0x72, 0 },
	{ "f24", 0x87, 0 },
	{ "f23", 0x86, 0 },
	{ "f22", 0x85, 0 },
	{ "f21", 0x84, 0 },
	{ "f20", 0x83, 0 },
	{ "f2", 0x71, 0 },
	{ "f19", 0x82, 0 },
	{ "f18", 0x81, 0 },
	{ "f17", 0x80, 0 },
	{ "f16", 0x7f, 0 },
	{ "f15", 0x7e, 0 },
	{ "f14", 0x7d, 0 },
	{ "f13", 0x7c, 0 },
	{ "f12", 0x7b, 0 },
	{ "f11", 0x7a, 0 },
	{ "f10", 0x79, 0 },
	{ "f", 0x46, 0 },
	{ "escape", 0x1b, 0 },
	{ "esc", 0x1b, 0 },
	{ "enter", 0x0d, 0 },
	{ "end", 0x23, 0 },
	{ "e", 0x45, 0 },
	{ "down", 0x28, 0 },
	{ "delete", 0x2e, 0 },
	{ "del", 0x2e, 0 },
	{ "d", 0x44, 0 },
	{ "ctrl", VK_CONTROL, 0 },
	{ "convert", 0x1c },
	{ "capslock", 0x14, 0 },
	{ "c", 0x43, 0 },
	{ "backspace", 0x08, 0 },
	{ "b", 0x42, 0 },
	{ "apps", VK_APPS, 0 },
	{ "alt", VK_MENU, 0 },
	{ "a", 0x41, 0 },
	{ "`", 0xC0, VK_SHIFT },
	{ "_", 0xE2, VK_SHIFT },
	{ "^", 0xDE, 0 },
	{ "]", 0xDD, 0 },
	{ "\\t", VK_TAB, 0 },
	{ "\\r\\n", VK_RETURN, 0 },
	{ "\\n", VK_RETURN, 0 },
	{ "\\", 0xDC, 0 },
	{ "\"", 0x32, VK_SHIFT },
	{ "[", 0xDB, 0 },
	{ "@", 0xC0, 0 },
	{ "?", 0xBF, VK_SHIFT },
	{ ">", 0xBE, VK_SHIFT },
	{ "=", 0xBD, VK_SHIFT },
	{ "<", 0xBC, VK_SHIFT },
	{ ";", 0xBB, 0 },
	{ ":", 0xBA, 0 },
	{ "9", 0x39, 0 },
	{ "8", 0x38, 0 },
	{ "7", 0x37, 0 },
	{ "6", 0x36, 0 },
	{ "5", 0x35, 0 },
	{ "4", 0x34, 0 },
	{ "3", 0x33, 0 },
	{ "2", 0x32, 0 },
	{ "1", 0x31, 0 },
	{ "0", 0x30, 0 },
	{ "/", 0xBF, 0 },
	{ ".", 0xBE, 0 },
	{ "-", 0xBD, 0 },
	{ ",", 0xBC, 0 },
	{ "+", 0xBB, VK_SHIFT },
	{ "*", 0xBA, VK_SHIFT },
	{ ")", 0x39, VK_SHIFT },
	{ "(", 0x38, VK_SHIFT },
	{ "'", 0x37, VK_SHIFT },
	{ "&", 0x36, VK_SHIFT },
	{ "%", 0x35, VK_SHIFT },
	{ "$", 0x34, VK_SHIFT },
	{ "#", 0x33, VK_SHIFT },
	{ "!", 0x31, VK_SHIFT },
	{ " ", VK_SPACE, 0 },
};

VKScanner::VKScanner(const std::string& input) : mInput(input), mPos(0)
{
	// ToDo:小文字にする
	std::transform(mInput.begin(), mInput.end(), mInput.begin(), ::tolower);

}

VKScanner::~VKScanner()
{
}

bool VKScanner::Get(KEY_DEFINE* key) const
{
	return Get(key, nullptr);
}

bool VKScanner::HasNext() const
{
	return Get(nullptr);
}

bool VKScanner::Next(KEY_DEFINE* key)
{
	return Get(key, &mPos);
}

bool VKScanner::Get(KEY_DEFINE* key, size_t* nextPos) const
{
	auto pos = mPos;
	while(pos != mInput.size()) {

		for (auto k : keyDefines) {
			size_t len = std::strlen(k.mKeyName);

			if (pos + len <= mInput.size() && mInput.compare(pos, len, k.mKeyName) == 0) {
				pos += len;
				if (key) { *key = k; }
				if (nextPos) { *nextPos = pos; }
				return true;
			}
		}

		// 不明な文字はスキップ
		pos++;
	}

	if (nextPos) { *nextPos = pos; }
	return false;
}
