#include "pch.h"
#include "OfficeFavorites.h"
#include "utility/RegistryKey.h"
#include "utility/Path.h"
#include "framework.h"
#include <shlwapi.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <mutex>
#include <iterator>

#pragma comment(lib, "Shlwapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using json = nlohmann::json;

namespace launcherapp {
namespace commands {
namespace office_favorites {

namespace {

// 対象となるアプリの一覧(レジストリのキー名とJSONのappフィールドで共通)
struct AppInfo
{
	OfficeAppType type;
	LPCTSTR name;
};
const AppInfo APP_INFOS[] = {
	{OfficeAppType::Excel, _T("Excel")},
	{OfficeAppType::Word, _T("Word")},
	{OfficeAppType::PowerPoint, _T("PowerPoint")},
};

// ピン止めされていることを示すレジストリ値の接頭辞
const LPCTSTR PINNED_PREFIX = _T("[F00000001]");

// Office 2016 以降のバージョン番号
const LPCTSTR OFFICE_VERSION = _T("16.0");

// URLをデコードするためのバッファサイズ(TCHAR単位)
const DWORD URL_BUFFER_SIZE = 2084;

// aggmru 配下のJSONファイル名のプレフィックス(アプリごと)
bool IsTargetAggMruFileName(const CString& fileName)
{
	// x: Excel, w: Word, p: PowerPoint(言語名は環境によって異なるためパターンで判定)
	bool hasPrefix = fileName.Left(7) == _T("x-mru4-") || fileName.Left(7) == _T("w-mru4-") || fileName.Left(7) == _T("p-mru4-");
	bool hasSuffix = fileName.Right(8) == _T("-sr.json");
	return hasPrefix && hasSuffix;
}

// URL の末尾要素をパーセントデコードして取得する(クエリ文字列は除く)
CString DecodeUrlLastSegment(const CString& url)
{
	// クエリ文字列(?以降)は表示名に含めない
	CString path = url;
	int queryPos = path.Find(_T('?'));
	if (queryPos >= 0) {
		path = path.Left(queryPos);
	}

	int pos = path.ReverseFind(_T('/'));
	CString segment = pos >= 0 ? path.Mid(pos + 1) : path;
	return OfficeFavorites::DecodeUrl(segment);
}

// 拡張子をドット付きに整える(空の場合は空を返す)
CString NormalizeExtension(const std::string& ext)
{
	if (ext.empty()) {
		return CString();
	}
	CString tmp;
	UTF2UTF(ext, tmp);
	if (tmp.Left(1) == _T(".")) {
		return tmp;
	}
	return _T(".") + tmp;
}

// レジストリからローカルファイルのお気に入りを取得する
std::vector<OfficeFavoriteItem> LoadRegistryItems()
{
	std::vector<OfficeFavoriteItem> items;

	RegistryKey HKCU(HKEY_CURRENT_USER);
	for (auto& app : APP_INFOS) {
		// User MRU 配下にはアカウントごとのキーがあるため、すべて走査する
		CString userMruPath;
		userMruPath.Format(_T("Software\\Microsoft\\Office\\%s\\%s\\User MRU"), OFFICE_VERSION, app.name);

		RegistryKey userMru;
		if (HKCU.OpenSubKey(userMruPath, userMru) == false) {
			continue;
		}

		std::vector<CString> accountKeys;
		userMru.EnumSubKeyNames(accountKeys);
		for (auto& accountKey : accountKeys) {
			CString fileMruPath = accountKey + _T("\\File MRU");

			std::vector<CString> valueNames;
			if (userMru.EnumValueNames(fileMruPath, valueNames) == false) {
				continue;
			}

			for (auto& valueName : valueNames) {
				// "Item N" の値のみを対象とする
				if (valueName.Left(5) != _T("Item ")) {
					continue;
				}

				CString value;
				if (userMru.GetValue(fileMruPath, valueName, value) == false) {
					continue;
				}

				// ピン止めされていないものは対象外
				CString path;
				if (OfficeFavorites::ParseRegistryValue(value, path) == false) {
					continue;
				}
				// ファイルが存在しない場合は保持しない
				if (Path::FileExists(path) == FALSE) {
					continue;
				}

				OfficeFavoriteItem item;
				item.mPath = path;
				item.mAppType = app.type;
				items.push_back(item);
			}
		}
	}

	return items;
}

// aggmru の JSON からSharePoint/OneDrive上のお気に入りを取得する
std::vector<OfficeFavoriteItem> LoadJsonItems()
{
	std::vector<OfficeFavoriteItem> items;

	std::vector<CString> files;
	OfficeFavorites::EnumAggMruJsonFiles(files);

	for (auto& file : files) {
		std::ifstream f(file, std::ios::binary);
		if (f.is_open() == false) {
			continue;
		}
		std::string text((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
		OfficeFavorites::ParseAggMruJson(text, items);
	}
	return items;
}

} // end of unnamed namespace

struct OfficeFavorites::PImpl
{
	std::mutex mMutex;

	// レジストリから取得した一覧
	std::vector<OfficeFavoriteItem> mRegistryItems;
	// JSONから取得した一覧
	std::vector<OfficeFavoriteItem> mJsonItems;
};

OfficeFavorites::OfficeFavorites() : in(std::make_unique<PImpl>())
{
}

OfficeFavorites::~OfficeFavorites()
{
}

// レジストリの情報を読み直す
void OfficeFavorites::ReloadRegistry()
{
	// 先に新しい一覧を作り、swapの瞬間だけロックする
	auto items = LoadRegistryItems();

	std::lock_guard<std::mutex> lock(in->mMutex);
	in->mRegistryItems.swap(items);
}

// JSONの情報を読み直す
void OfficeFavorites::ReloadJson()
{
	auto items = LoadJsonItems();

	std::lock_guard<std::mutex> lock(in->mMutex);
	in->mJsonItems.swap(items);
}

// 保持している情報をすべて破棄する
void OfficeFavorites::Clear()
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	in->mRegistryItems.clear();
	in->mJsonItems.clear();
}

// 保持しているお気に入りの一覧を取得する
std::vector<OfficeFavoriteItem> OfficeFavorites::GetItems()
{
	std::lock_guard<std::mutex> lock(in->mMutex);

	std::vector<OfficeFavoriteItem> items;
	items.reserve(in->mRegistryItems.size() + in->mJsonItems.size());
	items.insert(items.end(), in->mRegistryItems.begin(), in->mRegistryItems.end());
	items.insert(items.end(), in->mJsonItems.begin(), in->mJsonItems.end());
	return items;
}

/**
 * @brief レジストリの File MRU の値からピン止めされたパスを取り出す
 * @note 値の形式は [F00000001][T...][O...]*パス
 */
bool OfficeFavorites::ParseRegistryValue(const CString& value, CString& path)
{
	// ピン止めの有無を先頭の [F...] で判定する
	if (value.Left((int)_tcslen(PINNED_PREFIX)) != PINNED_PREFIX) {
		return false;
	}

	// パスは '*' の後に記述されている
	int pos = value.Find(_T('*'));
	if (pos < 0) {
		return false;
	}

	CString tmp = value.Mid(pos + 1);
	if (tmp.IsEmpty()) {
		return false;
	}
	path = tmp;
	return true;
}

/**
 * @brief aggmru の JSON 文字列からピン止めされた項目を取り出す
 */
bool OfficeFavorites::ParseAggMruJson(const std::string& text, std::vector<OfficeFavoriteItem>& items)
{
	try {
		json root = json::parse(text);
		auto& list = root["documents"]["items"];
		if (list.is_array() == false) {
			return false;
		}

		CString url;
		CString app;
		CString title;

		for (auto& entry : list) {
			// ピン止めされていない項目は対象外
			if (entry.contains("is_pinned") == false || entry["is_pinned"].is_boolean() == false) {
				continue;
			}
			if (entry["is_pinned"].get<bool>() == false) {
				continue;
			}

			// 対象アプリ(Excel/Word/PowerPoint)以外は対象外
			UTF2UTF(entry.value("app", std::string()), app);
			OfficeAppType appType{};
			if (AppTypeFromName(app, appType) == false) {
				continue;
			}

			UTF2UTF(entry.value("url", std::string()), url);
			if (url.IsEmpty()) {
				continue;
			}

			// 表示名は「title + 拡張子」で組み立てる(title が無い場合は空のままにし、パスから求める)
			UTF2UTF(entry.value("title", std::string()), title);
			CString extension = NormalizeExtension(entry.value("extension", std::string()));

			OfficeFavoriteItem item;
			item.mPath = url;
			item.mAppType = appType;
			if (title.IsEmpty() == FALSE) {
				item.mName = title + extension;
			}
			items.push_back(item);
		}
	}
	catch (const json::exception& e) {
		CString what;
		UTF2UTF(e.what(), what);
		spdlog::warn(_T("failed to parse office favorites json. {0}"), (LPCTSTR)what);
		return false;
	}
	return true;
}

/**
 * @brief aggmru 配下の対象 JSON ファイルを列挙する
 */
void OfficeFavorites::EnumAggMruJsonFiles(std::vector<CString>& files)
{
	std::vector<CString> tmp;

	CString base;
	DWORD len = GetEnvironmentVariable(_T("LOCALAPPDATA"), nullptr, 0);
	if (len == 0) {
		return;
	}
	std::vector<TCHAR> localAppData(len);
	GetEnvironmentVariable(_T("LOCALAPPDATA"), localAppData.data(), len);
	base.Format(_T("%s\\Microsoft\\Office\\%s\\aggmru\\"), localAppData.data(), OFFICE_VERSION);

	// aggmru 直下のアカウント別フォルダを走査する
	WIN32_FIND_DATA dirData{};
	HANDLE hDir = FindFirstFile(base + _T("*"), &dirData);
	if (hDir == INVALID_HANDLE_VALUE) {
		return;
	}

	CString dirPath;
	CString dirName;
	CString fileName;
	do {
		if ((dirData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			continue;
		}
		dirName = dirData.cFileName;
		if (dirName == _T(".") || dirName == _T("..")) {
			continue;
		}

		// フォルダ内の対象JSONを探す
		dirPath = base + dirName + _T("\\");
		WIN32_FIND_DATA fileData{};
		HANDLE hFind = FindFirstFile(dirPath + _T("*.json"), &fileData);
		if (hFind == INVALID_HANDLE_VALUE) {
			continue;
		}
		do {
			fileName = fileData.cFileName;
			if (IsTargetAggMruFileName(fileName)) {
				tmp.push_back(dirPath + fileName);
			}
		} while (FindNextFile(hFind, &fileData));
		FindClose(hFind);

	} while (FindNextFile(hDir, &dirData));
	FindClose(hDir);

	files.insert(files.end(), tmp.begin(), tmp.end());
}

/**
 * @brief アプリ名からアプリ種別を取得する
 */
bool OfficeFavorites::AppTypeFromName(const CString& appName, OfficeAppType& appType)
{
	for (auto& app : APP_INFOS) {
		if (appName == app.name) {
			appType = app.type;
			return true;
		}
	}
	return false;
}

/**
 * @brief アプリ種別からアプリ名を取得する
 */
CString OfficeFavorites::AppTypeName(OfficeAppType appType)
{
	for (auto& app : APP_INFOS) {
		if (app.type == appType) {
			return app.name;
		}
	}
	return CString();
}

/**
 * @brief アプリ種別に対応する実行ファイルのパスを App Paths から取得する
 */
bool OfficeFavorites::GetAppExePath(OfficeAppType appType, CString& exePath)
{
	CString exeName;
	switch (appType) {
	case OfficeAppType::Excel:
		exeName = _T("excel.exe");
		break;
	case OfficeAppType::Word:
		exeName = _T("winword.exe");
		break;
	case OfficeAppType::PowerPoint:
		exeName = _T("powerpnt.exe");
		break;
	default:
		return false;
	}

	// App Paths の既定値に実行ファイルのパスが登録されている
	CString subKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\") + exeName;
	RegistryKey HKLM(HKEY_LOCAL_MACHINE);
	CString value;
	if (HKLM.GetValue(subKey, _T(""), value) == false) {
		return false;
	}
	value.Trim(_T("\""));
	exePath = value;
	return true;
}

/**
 * @brief レジストリで監視対象となる User MRU のキーパスを取得する
 */
void OfficeFavorites::GetUserMRUKeyPaths(std::vector<CString>& keyPaths)
{
	CString path;
	for (auto& app : APP_INFOS) {
		path.Format(_T("Software\\Microsoft\\Office\\%s\\%s\\User MRU"), OFFICE_VERSION, app.name);
		keyPaths.push_back(path);
	}
}

/**
 * @brief URLか(http:// または https:// で始まるか)
 */
bool OfficeFavoriteItem::IsURL() const
{
	// スキームは大文字小文字を区別しない
	if (mPath.Left(7).CompareNoCase(_T("http://")) == 0) {
		return true;
	}
	return mPath.Left(8).CompareNoCase(_T("https://")) == 0;
}

/**
 * @brief 表示名(ファイル名)を取得する
 */
CString OfficeFavoriteItem::GetName() const
{
	// JSON の title から組み立てた表示名を優先する
	if (mName.IsEmpty() == FALSE) {
		return mName;
	}
	if (IsURL()) {
		return DecodeUrlLastSegment(mPath);
	}
	return CString(PathFindFileName(mPath));
}

/**
 * @brief 拡張子(ドット付き)を取得する
 */
CString OfficeFavoriteItem::GetExtension() const
{
	// 表示名から拡張子を求める(表示名は title + 拡張子 で組み立てたものを優先する)
	CString name = GetName();
	LPCTSTR ext = PathFindExtension(name);
	return CString(ext);
}

/**
 * @brief URL をパーセントデコードする
 */
CString OfficeFavorites::DecodeUrl(const CString& url)
{
	// UrlUnescape は入力も非const のため、コピーに対して処理する
	CString src = url;
	std::vector<TCHAR> buf(URL_BUFFER_SIZE, _T('\0'));
	DWORD len = (DWORD)buf.size();
	HRESULT hr = UrlUnescape(src.GetBuffer(), buf.data(), &len, URL_UNESCAPE_AS_UTF8);
	src.ReleaseBuffer();
	if (hr != S_OK) {
		return url;
	}
	return CString(buf.data());
}

} // end of namespace office_favorites
} // end of namespace commands
} // end of namespace launcherapp
