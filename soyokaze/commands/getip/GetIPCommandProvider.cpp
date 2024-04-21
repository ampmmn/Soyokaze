#include "pch.h"
#include "GetIPCommandProvider.h"
#include "commands/getip/GetIPCommand.h"
#include "commands/core/CommandRepository.h"

#include <ws2tcpip.h>
#pragma comment(lib, "iphlpapi.lib")

#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace getip {


using CommandRepository = launcherapp::core::CommandRepository;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(GetIPCommandProvider)


GetIPCommandProvider::GetIPCommandProvider()
{
}

GetIPCommandProvider::~GetIPCommandProvider()
{
}

CString GetIPCommandProvider::GetName()
{
	return _T("GetIPAddress");
}

// 一時的なコマンドを必要に応じて提供する
void GetIPCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	int level = pattern->Match(_T("getip"));
	if (level != Pattern::WholeMatch) {
		return;
	}

	DWORD size;
	if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &size) != ERROR_BUFFER_OVERFLOW) {
		return ;
	}

	std::vector<BYTE> data(size);
	PIP_ADAPTER_ADDRESSES pAdapterAddresses = (PIP_ADAPTER_ADDRESSES)&data.front();
	if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAdapterAddresses, &size) != ERROR_SUCCESS) {
		return;
	}

	PIP_ADAPTER_ADDRESSES addr = pAdapterAddresses;
	for (; addr; addr = addr->Next) {

		// addr->FriendlyName  : 表示名
		LPCTSTR displayName = addr->FriendlyName;

		IP_ADAPTER_UNICAST_ADDRESS* uniAddr = addr->FirstUnicastAddress;
		if (uniAddr == nullptr) {
			continue;
		}

		for(;uniAddr; uniAddr = uniAddr->Next) {

			if (!(uniAddr->Flags & IP_ADAPTER_ADDRESS_DNS_ELIGIBLE)) {
				continue;
			}

			SOCKET_ADDRESS& addr = uniAddr->Address;
			sockaddr* sa = addr.lpSockaddr;

			if (sa->sa_family == AF_INET) {
				char ipStr[16];
				inet_ntop(AF_INET, &((struct sockaddr_in *)sa)->sin_addr, ipStr, sizeof(ipStr));
				commands.push_back(CommandQueryItem(Pattern::WholeMatch, new GetIPCommand(displayName, CString(CStringA(ipStr)))));
			}
			else if (sa->sa_family == AF_INET6) {

				auto sa6 = (struct sockaddr_in6*)sa;
				int sz = 40;
				char ipStr[40];
				inet_ntop(sa6->sin6_family, &sa6->sin6_addr, ipStr, sz);
				commands.push_back(CommandQueryItem(Pattern::WholeMatch, new GetIPCommand(displayName, CString(CStringA(ipStr)))));
			}
		}
	}
}


} // end of namespace getip
} // end of namespace commands
} // end of namespace launcherapp

