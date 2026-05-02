#pragma once

#include <memory>


class LeftBorderLabel
{
public:
	LeftBorderLabel();
	~LeftBorderLabel();

	static bool Initialize();
	void Attach(HWND h);

	void SetBorderWidth(int cx);
	void SetBorderMargin(int cx);
	void SetBorderColor(COLORREF cr);

private:
	void Draw();
	void OnPaint();
	void OnSize(UINT type, int cx, int cy);

	static LRESULT CALLBACK WindowProc(HWND h, UINT msg, WPARAM wp, LPARAM lp);
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

