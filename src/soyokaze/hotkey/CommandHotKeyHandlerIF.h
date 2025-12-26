#pragma once

namespace launcherapp {
namespace core {

class CommandHotKeyHandler
{
public:
	virtual ~CommandHotKeyHandler() {}

	virtual CString GetDisplayName() = 0;
	virtual bool Invoke() = 0;
	virtual bool IsTemporaryHandler() = 0;

	virtual uint32_t AddRef() = 0;
	virtual uint32_t Release() = 0;

};

}
}
