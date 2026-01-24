#pragma once

#include <memory>

class ManualWindow
{
	ManualWindow();
	~ManualWindow();

public:
	static ManualWindow* GetInstance();
	void Open(const CString& url);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

