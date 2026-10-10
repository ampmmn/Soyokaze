#pragma once


class RegistryKey
{
public:
	RegistryKey();
	RegistryKey(HKEY hKey);
	~RegistryKey();

	// 二重に RegCloseKey されないようコピーは禁止し、ムーブのみ許可する
	RegistryKey(const RegistryKey&) = delete;
	RegistryKey& operator=(const RegistryKey&) = delete;
	RegistryKey(RegistryKey&& rhs) noexcept;
	RegistryKey& operator=(RegistryKey&& rhs) noexcept;

	// 保持しているハンドルを取得する(RegNotifyChangeKeyValue などに渡すため)
	HKEY GetHandle() const;

	bool EnumSubKeyNames(LPCTSTR subKey, std::vector<CString>& subKeyNames);
	bool EnumSubKeyNames(std::vector<CString>& subKeyNames);
	bool EnumValueNames(LPCTSTR subKey, std::vector<CString>& valueNames);
	bool EnumValueNames(std::vector<CString>& subKeyNames);
	bool OpenSubKey(LPCTSTR subKeyName, RegistryKey& subKey);

	bool GetValue(LPCTSTR subKey, LPCTSTR valueName, CString& value);
	bool GetValue(LPCTSTR subKey, LPCTSTR valueName, DWORD& value);
	bool GetValue(LPCTSTR valueName, CString& value);

private:
	void Close();

private:
	HKEY mKey;
};
