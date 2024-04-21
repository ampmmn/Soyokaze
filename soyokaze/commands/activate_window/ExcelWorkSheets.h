#pragma once

#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace activate_window {

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
	Worksheet(const CString& appPath, const CString& workbookName, const CString& sheetName);
	~Worksheet();

	const CString& GetAppPath();
	const CString& GetWorkbookName();
	const CString& GetSheetName();

	BOOL Activate(bool isShowMaximize = false);

	uint32_t AddRef();
	uint32_t Release();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

