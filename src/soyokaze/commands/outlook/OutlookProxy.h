#pragma once 

#include "commands/outlook/OutlookFolderQueryResult.h"

namespace launcherapp { namespace commands { namespace outlook {

class OutlookProxy
{
	OutlookProxy();
	~OutlookProxy();

public:
	static OutlookProxy* GetInstance();

	bool Initialize();

	bool IsAppRunning();
	bool EnumFolders(std::vector<QueryResult>& results);
	bool SelectFolder(CComPtr<IDispatch> folder);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}}

