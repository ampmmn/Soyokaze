#include "pch.h"
#include "framework.h"
#include "utility/AppProfile.h"

static const CString& getLogFilePath()
{
	static CString filePath;
	if (filePath.IsEmpty() == FALSE) {
		return filePath;
	}

	TCHAR path[32768];
	CAppProfile::GetDirPath(path, 32768);
	PathAppend(path, _T("soyokaze.log"));

	filePath = path;
	return filePath;
}

void SoyokazeInfo(LPCTSTR fmt, ...)
{
}

void SoyokazeLog(LPCTSTR fmt, ...)
{
	auto& path= getLogFilePath();
	FILE* fp = nullptr;
	if (_tfopen_s(&fp, path, _T("a")) != 0) {
		return ;
	}

    
	va_list args;
	va_start (args, fmt);
   
	_vftprintf(fp, fmt, args);

	va_end(args);

	fwrite("\n", 1, 1, fp); 

	fclose(fp);
}

void SoyokazeWarning(LPCTSTR fmt, ...)
{
}

void SoyokazeError(LPCTSTR fmt, ...)
{
}

