#include "pch.h"
#include "RandomMacro.h"
#include <rpcdce.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Rpcrt4.lib")

namespace launcherapp {
namespace macros {
namespace random {

REGISTER_LAUNCHERMACRO(RandomMacro)

RandomMacro::RandomMacro()
{
	mName = _T("random");
}

RandomMacro::~RandomMacro()
{
}

bool RandomMacro::Evaluate(const std::vector<CString>& args, CString& result)
{
	if (args.size() == 0) {
		return false;
	}

	if (args[0].CompareNoCase(_T("uuid")) == 0) {
		UUID uuid;
		auto ret = UuidCreate(&uuid);
		if (ret != RPC_S_OK) {
			spdlog::warn(_T("Failed to UuidCreate!"));
			return false;
		}

		RPC_WSTR strUuid;
		ret = UuidToString(&uuid, &strUuid);
		if (ret != RPC_S_OK) {
			spdlog::warn(_T("Failed to UuidToString!"));
			return false;
		}

		result = (const wchar_t*)strUuid;

		RpcStringFree(&strUuid);

		return true;
	}

	return false;

}


}
}
}
