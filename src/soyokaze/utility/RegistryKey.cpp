#include "pch.h"
#include "RegistryKey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/**
 * @brief デフォルトコンストラクタ
 */
RegistryKey::RegistryKey() : mKey(nullptr)
{
}

/**
 * @brief HKEY を受け取るコンストラクタ
 * @param hKey レジストリキーのハンドル
 */
RegistryKey::RegistryKey(HKEY hKey) : mKey(hKey)
{
}

/**
 * @brief デストラクタ
 */
RegistryKey::~RegistryKey()
{
	if (mKey == nullptr) {
		return;
	}

	// HKEY_LOCAL_MACHINE, HKEY_CLASSES_ROOT, HKEY_CURRENT_USER 以外のキーを閉じる
	bool isHKLM = mKey == HKEY_LOCAL_MACHINE;
	bool isHKCR = mKey == HKEY_CLASSES_ROOT;
	bool isHKCU = mKey == HKEY_CURRENT_USER;
	if (isHKLM == false && isHKCR == false && isHKCU == false) {
		// 他の定義済みキーは今のところ使ってないので見ない
		RegCloseKey(mKey);
		mKey = nullptr;
	}
}

/**
 * @brief サブキーの名前を列挙する
 * @param subKey サブキーの名前
 * @param subKeyNames サブキーの名前を格納するベクター
 * @return 列挙に成功した場合は true、失敗した場合は false
 */
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

/**
 * @brief サブキーの名前を列挙する（オーバーロード）
 * @param subKeyNames サブキーの名前を格納するベクター
 * @return 列挙に成功した場合は true、失敗した場合は false
 */
bool RegistryKey::EnumSubKeyNames(std::vector<CString>& subKeyNames)
{
	if (mKey == nullptr) {
		return false;
	}
	std::vector<CString> tmp;
	for (int index = 0; ; ++index) {
		TCHAR buf[1024] = { _T('\0') };
		DWORD len = 1024;
		// サブキーの名前を列挙
		if (RegEnumKey(mKey, index, buf, len) != ERROR_SUCCESS) {
			break;
		}
		tmp.push_back(buf);
	}

	tmp.swap(subKeyNames);
	return true;
}

/**
 * @brief 値の名前を列挙する
 * @param subKey サブキーの名前
 * @param valueNames 値の名前を格納するベクター
 * @return 列挙に成功した場合は true、失敗した場合は false
 */
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

/**
 * @brief 値の名前を列挙する（オーバーロード）
 * @param valueNames 値の名前を格納するベクター
 * @return 列挙に成功した場合は true、失敗した場合は false
 */
bool RegistryKey::EnumValueNames(std::vector<CString>& valueNames)
{
	if (mKey == nullptr) {
		return false;
	}

	std::vector<CString> tmp;
	for (int index = 0; ; ++index) {
		TCHAR buf[1024] = { _T('\0') };
		DWORD len = 1024;
		// 値の名前を列挙
		if (RegEnumValue(mKey, index, buf, &len, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
			break;
		}
		tmp.push_back(buf);
	}

	tmp.swap(valueNames);
	return true;
}

/**
 * @brief サブキーを開く
 * @param subKeyName サブキーの名前
 * @param subKey 開いたサブキーを格納する RegistryKey オブジェクト
 * @return 開くのに成功した場合は true、失敗した場合は false
 */
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

/**
 * @brief レジストリ値を取得する
 * @param subKey サブキーの名前
 * @param valueName 値の名前
 * @param value 取得した値を格納する CString オブジェクト
 * @return 取得に成功した場合は true、失敗した場合は false
 */
bool RegistryKey::GetValue(LPCTSTR subKey, LPCTSTR valueName, CString& value)
{
	std::vector<TCHAR> buff(MAX_PATH_NTFS);
	DWORD len = MAX_PATH_NTFS;
	// レジストリ値を取得
	if (RegGetValue(mKey, subKey, valueName, RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ, nullptr, &buff.front(), &len) != ERROR_SUCCESS) {
		return false;
	}

	value = &buff.front();
	return true;
}

/**
 * @brief レジストリ値を取得する
 * @param subKey サブキーの名前
 * @param valueName 値の名前
 * @param value 取得した値を格納するDWORD
 * @return 取得に成功した場合は true、失敗した場合は false
 */
bool RegistryKey::GetValue(LPCTSTR subKey, LPCTSTR valueName, DWORD& value)
{
	DWORD len = sizeof(value);
	// レジストリ値を取得
	if (RegGetValue(mKey, subKey, valueName, RRF_RT_REG_DWORD, nullptr, &value, &len) != ERROR_SUCCESS) {
		return false;
	}
	return true;
}

/**
 * @brief レジストリ値を取得する（オーバーロード）
 * @param valueName 値の名前
 * @param value 取得した値を格納する CString オブジェクト
 * @return 取得に成功した場合は true、失敗した場合は false
 */
bool RegistryKey::GetValue(LPCTSTR valueName, CString& value)
{
	return GetValue(nullptr, valueName, value);
}

