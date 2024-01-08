#pragma once

#include "commands/simple_dict/SimpleDictCommandUpdateListenerIF.h"
#include <memory>
#include <vector>
#include "Pattern.h"

namespace soyokaze {
namespace commands {
namespace simple_dict {

class SimpleDictCommand;

class SimpleDictDatabase : public CommandUpdateListenerIF
{
public:
	struct ITEM
	{
		CString mRecord;
	};

public:
	SimpleDictDatabase();
	~SimpleDictDatabase();

	void Query(Pattern* pattern, std::vector<ITEM>& items, int limit, DWORD timeout);

	void OnUpdateCommand(SimpleDictCommand* cmd) override;
	void OnDeleteCommand(SimpleDictCommand* cmd) override;
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

