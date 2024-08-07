#include "pch.h"
#include "RegistryKey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RegistryKey::RegistryKey() : mKey(nullptr)
{
}

RegistryKey::RegistryKey(HKEY hKey) : mKey(hKey)
{
}

RegistryKey::~RegistryKey()
{
	if (mKey == nullptr) {
		return ;
	}

	bool isHKLM = mKey == HKEY_LOCAL_MACHINE;
	bool isHKCR = mKey == HKEY_CLASSES_ROOT;
	bool isHKCU = mKey == HKEY_CURRENT_USER;
	if (isHKLM == false && isHKCR == false && isHKCU == false) {
		// 他の定義済みキーは今のところ使ってないので見ない
		RegCloseKey(mKey);
		mKey = nullptr;
	}
}

bool RegistryKey::EnumSubKeyNames(LPCTSTR subKey, std::vector<CString>& subKeyNames)
{
	if (mKey == nullptr) {
		return false;
	}

	RegistryKey subKeyObj;
	if (RegOpenKey(mKey, subKey, &(subKeyObj.mKey)) != ERROR_SUCCESS) {
		return false;
	}
	return subKeyObj.EnumSubKeyNames(subKeyNames);
}

bool RegistryKey::EnumSubKeyNames(std::vector<CString>& subKeyNames)
{
	if (mKey == nullptr) {
		return false;
	}
	std::vector<CString> tmp;
	for (int index = 0; ; ++index) {
		TCHAR buf[1024] = { _T('\0') };
		DWORD len = 1024;
		if (RegEnumKey(mKey, index, buf, len) != ERROR_SUCCESS) {
			break;
		}
		tmp.push_back(buf);
	}

	tmp.swap(subKeyNames);
	return true;
}

bool RegistryKey::EnumValueNames(LPCTSTR subKey, std::vector<CString>& valueNames)
{
	if (mKey == nullptr) {
		return false;
	}

	RegistryKey subKeyObj;
	if (RegOpenKey(mKey, subKey, &(subKeyObj.mKey)) != ERROR_SUCCESS) {
		return false;
	}
	return subKeyObj.EnumValueNames(valueNames);
}

bool RegistryKey::EnumValueNames(std::vector<CString>& valueNames)
{
	if (mKey == nullptr) {
		return false;
	}

	std::vector<CString> tmp;
	for (int index = 0; ; ++index) {
		TCHAR buf[1024] = { _T('\0') };
		DWORD len = 1024;
		if (RegEnumValue(mKey, index, buf, &len, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
			break;
		}
		tmp.push_back(buf);
	}

	tmp.swap(valueNames);
	return true;
}

bool RegistryKey::OpenSubKey(LPCTSTR subKeyName, RegistryKey& subKey)
{
	if (mKey == nullptr) {
		return false;
	}

	HKEY hkey = NULL;
	if (RegOpenKey(mKey, subKeyName, &hkey) != ERROR_SUCCESS) {
		return false;
	}

	if (subKey.mKey != nullptr) {
		RegCloseKey(subKey.mKey);
	}
	subKey.mKey = hkey;
	
	return true;
}

bool RegistryKey::GetValue(LPCTSTR subKey, LPCTSTR valueName, CString& value)
{
	std::vector<TCHAR> buff(MAX_PATH_NTFS);
	DWORD len = MAX_PATH_NTFS;
	if (RegGetValue(mKey, subKey, valueName, RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ, nullptr, &buff.front(), &len) != ERROR_SUCCESS) {
		return false;
	}

	value = &buff.front();
	return true;
}

bool RegistryKey::GetValue(LPCTSTR valueName, CString& value)
{
	return GetValue(nullptr, valueName, value);
}

