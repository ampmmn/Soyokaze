#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace simple_dict {

class ExcelApplication
{
public:
	ExcelApplication();
	~ExcelApplication();

public:
	int GetCellText(const CString wbPath, const CString& sheetName, const CString& address, std::vector<CString>& texts);

	static bool IsInstalled();
	static bool GetSelection(CString* wbPath, CString* sheetName, CString* address);

private:
	void Quit();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

