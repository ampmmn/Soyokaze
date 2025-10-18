#pragma once

#include <memory>
#include <functional>

class ProxyWindow
{
private:
	ProxyWindow();
	~ProxyWindow();

public:
	static ProxyWindow* GetInstance();
	
	int Initialize();
	int Finalize();

	void Abort();
	bool IsAbort();

	bool RequestCallback(std::function<bool()> func);

private:
	static LRESULT CALLBACK OnWindowProc(HWND,UINT,WPARAM,LPARAM);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

