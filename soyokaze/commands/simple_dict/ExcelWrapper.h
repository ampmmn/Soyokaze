#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace simple_dict {

class ExcelApplication
{
public:
	ExcelApplication();
	ExcelApplication(bool isGetObject);
	~ExcelApplication();

public:
	bool IsInstalled();
	bool IsAvailable();

	bool GetFilePath(CString& filePath);
	CString GetActiveSheetName();
	CString GetSelectionAddress(int& cols, int& rows);

	int GetCellText(const CString wbPath, const CString& sheetName, const CString& address, std::vector<CString>& texts);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

