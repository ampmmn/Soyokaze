#pragma once

#include <memory>


class KeyInputWatch
{
	struct KEYEVENT;
public:
	KeyInputWatch();
	~KeyInputWatch();

	bool Create();

private:
	static LRESULT CALLBACK OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

