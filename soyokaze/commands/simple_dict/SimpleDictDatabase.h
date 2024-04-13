#pragma once

#include "commands/simple_dict/SimpleDictCommandUpdateListenerIF.h"
#include <memory>
#include <vector>
#include "matcher/Pattern.h"

namespace soyokaze {
namespace commands {
namespace simple_dict {

class SimpleDictCommand;

class SimpleDictDatabase : public CommandUpdateListenerIF
{
public:
	struct ITEM
	{
		ITEM(int level, const CString& name, const CString& key, const CString& value) :
		 	mName(name), mMatchLevel(level), mKey(key), mValue(value) {}
		ITEM(const ITEM&) = default;

		int mMatchLevel;
		CString mName;
		CString mKey;
		CString mValue;
	};

public:
	SimpleDictDatabase();
	~SimpleDictDatabase();

	void Query(Pattern* pattern, std::vector<ITEM>& items, int limit, DWORD timeout);

	void OnUpdateCommand(SimpleDictCommand* cmd, const CString& oldName) override;
	void OnDeleteCommand(SimpleDictCommand* cmd) override;
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

