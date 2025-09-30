#pragma once


class RegistryKey
{
public:
	RegistryKey();
	RegistryKey(HKEY hKey);
	~RegistryKey();

	bool EnumSubKeyNames(LPCTSTR subKey, std::vector<CString>& subKeyNames);
	bool EnumSubKeyNames(std::vector<CString>& subKeyNames);
	bool EnumValueNames(LPCTSTR subKey, std::vector<CString>& valueNames);
	bool EnumValueNames(std::vector<CString>& subKeyNames);
	bool OpenSubKey(LPCTSTR subKeyName, RegistryKey& subKey);

	bool GetValue(LPCTSTR subKey, LPCTSTR valueName, CString& value);
	bool GetValue(LPCTSTR subKey, LPCTSTR valueName, DWORD& value);
	bool GetValue(LPCTSTR valueName, CString& value);

private:
	HKEY mKey;
};
