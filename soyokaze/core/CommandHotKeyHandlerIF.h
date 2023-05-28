#pragma once

namespace soyokaze {
namespace core {

class CommandHotKeyHandler
{
public:
	virtual ~CommandHotKeyHandler() {}

	virtual CString GetDisplayName() = 0;
	virtual bool Invoke() = 0;

};

}
}
