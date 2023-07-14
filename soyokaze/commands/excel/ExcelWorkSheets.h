#pragma once

#include <memory>
#include <vector>

namespace soyokaze {
namespace commands {
namespace excel {

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
	Worksheet(const CString& workbookName, const CString& sheetName);
	~Worksheet();

	const CString& GetWorkbookName();
	const CString& GetSheetName();

	BOOL Activate();

	uint32_t AddRef();
	uint32_t Release();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



} // end of namespace excel
} // end of namespace commands
} // end of namespace soyokaze

