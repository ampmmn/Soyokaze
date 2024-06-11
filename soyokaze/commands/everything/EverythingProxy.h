#pragma once

#include <memory>
#include <vector>

#include "commands/everything/EverythingResult.h"

namespace launcherapp {
namespace commands {
namespace everything {

class EverythingProxy
{
private:
	EverythingProxy();
	~EverythingProxy();

public:
	static EverythingProxy* Get();

	void Query(const CString& queryStr, std::vector<EverythingResult>& results);
	bool ActivateMainWindow();

	bool IsUseAPI();
	bool IsUseWM();

	int GetLastMethod();

	HICON GetIcon();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

