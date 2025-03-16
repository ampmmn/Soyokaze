#pragma once

#include <memory>
#include "mainwindow/interprocess/InterProcessEventID.h"

namespace launcherapp {
namespace mainwindow {
namespace interprocess {

class InterProcessMessageQueue
{
	InterProcessMessageQueue();
	~InterProcessMessageQueue();

public:
	static InterProcessMessageQueue* GetInstance();
	bool Enqueue(EVENT_ID id, void* data, size_t len);
	bool Dequeue(EVENT_ID* id, std::vector<uint8_t>& stm);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

}
}
}

