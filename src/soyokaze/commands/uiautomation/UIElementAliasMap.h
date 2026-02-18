#pragma once

#include <memory>
#include <vector>

namespace launcherapp { namespace commands { namespace uiautomation {

class UIElementAliasMap
{
	UIElementAliasMap();
	~UIElementAliasMap();
public:
	static UIElementAliasMap* GetInstance();

	void Update();

	bool GetAlias(const CString& wndClassName, const CString& orgMenuName, UINT commandId, CString& alias);

	void EnumElements(const CString& wndClassName, std::vector<std::pair<UINT,CString> >& elements);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}} // end of namespace launcherapp::commands::uiautomation


