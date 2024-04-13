#pragma once

namespace soyokaze {
namespace commands {
namespace simple_dict {

class SimpleDictCommand;

class CommandUpdateListenerIF
{
public:
	virtual ~CommandUpdateListenerIF() {}

	virtual void OnUpdateCommand(SimpleDictCommand* cmd, const CString& oldName) = 0;
	virtual void OnDeleteCommand(SimpleDictCommand* cmd) = 0;
};


}
}
}
