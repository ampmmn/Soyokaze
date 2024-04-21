#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace pathconvert {

class P4AppSettings
{
public:
	struct ITEM {
		CString mPort;
		CString mUser;
		CString mClient;
	};
public:
	P4AppSettings();
	~P4AppSettings();

public:
	size_t GetCount();
	void GetItems(std::vector<ITEM>& items);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp


