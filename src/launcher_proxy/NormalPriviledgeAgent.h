#pragma once

#include <memory>

class NormalPriviledgeAgent
{
public:
	NormalPriviledgeAgent();
	~NormalPriviledgeAgent();

	int Run(HINSTANCE hInst);

private:
	static LRESULT CALLBACK OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
	void ProcRequest();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

