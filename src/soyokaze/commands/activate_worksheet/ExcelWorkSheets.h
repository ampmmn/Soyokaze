#pragma once

#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace activate_worksheet {

class Worksheet;

class WorkSheets
{
public:
	WorkSheets();
	~WorkSheets();

public:
	bool GetWorksheets(std::vector<Worksheet*>& worksheets);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


class Worksheet
{
public:
	Worksheet(const std::wstring& workbookName, const std::wstring& sheetName);
	~Worksheet();

	const std::wstring& GetWorkbookName();
	const std::wstring& GetSheetName();

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

