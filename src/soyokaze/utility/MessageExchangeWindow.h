#pragma once

#include <functional>
#include <memory>

class MessageExchangeWindow
{
public:
	MessageExchangeWindow();
	~MessageExchangeWindow();

	bool Create(LPCTSTR caption, std::function<LRESULT(HWND,UINT,WPARAM,LPARAM)> callback);
	bool Create(LPCTSTR caption = _T(""));
	void Destroy();

	void SetCallback(std::function<LRESULT(HWND,UINT,WPARAM,LPARAM)> callback);
	bool Post(UINT msg, WPARAM wp, LPARAM lp);

	bool Exists();
	HWND GetHwnd();

private:
	static LRESULT CALLBACK OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

