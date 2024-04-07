#pragma once

#include <memory>

class OperationWatcher
{
public:
	OperationWatcher();
	~OperationWatcher();

	void StartWatch(CWnd* wnd);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

