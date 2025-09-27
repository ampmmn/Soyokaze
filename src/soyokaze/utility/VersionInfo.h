#pragma once

class VersionInfo
{
public:
	static bool GetVersionInfo(CString& version);
	static bool GetVersionInfo(const CString& targetPath, CString& version);
	static bool GetProductName(const CString& targetPath, CString& productName);

	static bool GetBuildDateTime(CTime& tmBuildDate);
};

