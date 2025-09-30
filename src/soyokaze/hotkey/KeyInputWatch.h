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
	LRESULT OnTimerProc();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

