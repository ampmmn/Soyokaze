#pragma once

class VersionInfo
{
public:
	static bool GetVersionInfo(CString& version);
	static bool GetVersionInfo(const CString& targetPath, CString& version);
	static bool GetBuildDateTime(CTime& tmBuildDate);
};

