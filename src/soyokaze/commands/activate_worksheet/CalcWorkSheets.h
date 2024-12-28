#pragma once

#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace activate_worksheet {

class CalcWorksheet;

class CalcWorkSheets
{
public:
	CalcWorkSheets();
	~CalcWorkSheets();

public:
	bool GetWorksheets(std::vector<CalcWorksheet*>& worksheets);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


class CalcWorksheet
{
public:
	CalcWorksheet(const CString& workbookName, const CString& sheetName);
	~CalcWorksheet();

	const CString& GetWorkbookName();
	const CString& GetSheetName();

	BOOL Activate(bool isShowMaximize = false);

	uint32_t AddRef();
	uint32_t Release();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



} // end of namespace activate_worksheet
} // end of namespace commands
} // end of namespace launcherapp

