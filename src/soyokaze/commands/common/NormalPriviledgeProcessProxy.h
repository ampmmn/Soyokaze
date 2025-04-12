#pragma once

#include <memory>
#include <map>

namespace launcherapp {
namespace commands {
namespace common {

class NormalPriviledgeProcessProxy
{
	NormalPriviledgeProcessProxy();
	~NormalPriviledgeProcessProxy();

public:
	static NormalPriviledgeProcessProxy* GetInstance();

	bool StartProcess(SHELLEXECUTEINFO* si, const std::map<CString, CString>& envMap);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}
}


