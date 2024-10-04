#pragma once

#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace simple_dict {

class SimpleDictCommand;

class DictionaryLoader
{
	DictionaryLoader();
	~DictionaryLoader();
public:
	static DictionaryLoader* Get();
	void AddWaitingQueue(SimpleDictCommand* cmd);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

