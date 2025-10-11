#include "pch.h"
#include "Path.h"
#include "utility/AppProfile.h"
#include <thread>
#include <mutex>
#include <map>
#include <lm.h>

#pragma comment(lib, "Netapi32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


class Path::QueryData
{
public:
	QueryData() :
	 	mEventHandle(CreateEvent(nullptr, FALSE, FALSE, nullptr)), mRefCount(1), mResult(false)
 	{
	}
	~QueryData() {
		CloseHandle(mEventHandle);
	}

	void Push(bool result) {
		mResult = result;
		SetEvent(mEventHandle);
	}

	bool TryGet(DWORD timeout)
	{
		if (WaitForSingleObject(mEventHandle, timeout) == WAIT_TIMEOUT) {
			return false;
		}
		bool returnValue = mResult;
		return returnValue;
	}
	

	uint32_t AddRef() {
		return InterlockedIncrement(&mRefCount);
	}

	uint32_t Release() {
		auto n = InterlockedDecrement(&mRefCount);
		if (n == 0) {
			delete this;
		}
		return n;
	}

private:
	HANDLE mEventHandle;
	uint32_t mRefCount;
	bool mResult;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


class Path::QueryDataHost
{
private:
	QueryDataHost() {}
	~QueryDataHost() {}

public:
	static QueryDataHost* Get()
	{
		static QueryDataHost inst;
		return &inst;
	}

	QueryData* RequestFileExists(const tstring& pathStr)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mQueryMap.size() >= 64) {
			return nullptr;
		}

		tstring key(_T("FileExists_") + pathStr);

		auto it = mQueryMap.find(key);
		if (it != mQueryMap.end()) {
			auto query = it->second;
			query->AddRef();
			return query;
		}

		auto query = new QueryData();
		query->AddRef();
		mQueryMap[key] = query;

		std::thread th([pathStr, query]() {
				bool isFileExist = PathFileExists(pathStr.c_str());
				query->Push(isFileExist);

				QueryDataHost::Get()->ReleaseQuery(query);
		});
		th.detach();

		return query;
	}

	QueryData* RequestIsDirectory(const tstring& pathStr)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mQueryMap.size() >= 64) {
			return nullptr;
		}

		tstring key(_T("IsDirectory_") + pathStr);

		auto it = mQueryMap.find(key);
		if (it != mQueryMap.end()) {
			auto query = it->second;
			query->AddRef();
			return query;
		}

		auto query = new QueryData();
		query->AddRef();
		mQueryMap[key] = query;

		std::thread th([pathStr, query]() {
				bool isFileExist = PathIsDirectory(pathStr.c_str());
				query->Push(isFileExist);

				QueryDataHost::Get()->ReleaseQuery(query);
		});
		th.detach();

		return query;
	}

	void ReleaseQuery(QueryData* queue)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		for (auto it = mQueryMap.begin(); it != mQueryMap.end(); ++it) {
			if (it->second != queue) {
				continue;
			}

			it->second->Release();
			mQueryMap.erase(it);
			break;
		}
	}

private:
	std::map<tstring, QueryData*> mQueryMap;
	std::mutex mMutex;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


Path::Path() : mPath(MAX_PATH_NTFS)
{
}

Path::Path(LPCTSTR initPath) : mPath(MAX_PATH_NTFS)
{
	_tcsncpy_s(mPath.data(), (DWORD)mPath.size(), initPath, _TRUNCATE);
	std::replace(mPath.begin(), mPath.end(), _T('/'), _T('\\'));
}

Path::Path(MODULEFILEPATH_TAG tag, LPCTSTR extraPath) : mPath(MAX_PATH_NTFS)
{
	GetModuleFileName(nullptr, mPath.data(), (DWORD)mPath.size());
	if (tag == MODULEFILEDIR) {
		RemoveFileSpec();
	}
	if (extraPath != nullptr) {
		Append(extraPath);
	}
}

Path::Path(APPPROFILE_TAG tag, LPCTSTR extraPath) : mPath(MAX_PATH_NTFS)
{
	bool isPerMachine = (tag == APPDIRPERMACHINE);
	CAppProfile::GetDirPath(data(), (DWORD)size(), isPerMachine);
	if (extraPath != nullptr) {
		Append(extraPath);
	}
}

Path::Path(SYSTEMDIRECTORY_TAG tag, LPCTSTR extraPath) : mPath(MAX_PATH_NTFS)
{
	if (tag == SYSTEMDIR) {
		GetSystemDirectory(data(), (DWORD)size());
	}
	else if (tag == APPDATA) {
		size_t reqLen = 0;
		_tgetenv_s(&reqLen, data(), size(), _T("APPDATA"));
	}
	else if (tag == LOCALAPPDATA) {
		size_t reqLen = 0;
		_tgetenv_s(&reqLen, data(), size(), _T("LOCALAPPDATA"));
	}
	else if (tag == USERPROFILE) {
		size_t reqLen = 0;
		_tgetenv_s(&reqLen, data(), size(), _T("USERPROFILE"));
	}
	if (extraPath != nullptr) {
		Append(extraPath);
	}
}

Path::~Path()
{
}

Path& Path::operator = (const CString& path)
{
	mPath.resize(path.GetLength() + 1);
	_tcsncpy_s(mPath.data(), (DWORD)mPath.size(), path, _TRUNCATE);
	return *this;
}

bool Path::FileUriToLocalPath(const String& uri)
{
	static std::string prefix("file:///");
	if (uri.find(prefix) != 0) {
		return false;
	}

	std::string path(uri.substr(prefix.length()));
	HRESULT result = UrlUnescapeA((char*)path.data(), nullptr, nullptr, URL_UNESCAPE_AS_UTF8 | URL_UNESCAPE_INPLACE);
	if (result != S_OK) {
		return false;
	}

	for (auto& c : path) {
		if (c == '/') { c = '\\'; }
	}

	CString tmp;
	UTF2UTF(path, tmp);
	(*this) = tmp;

	return true;
}

bool Path::IsEmptyPath() const
{
	return mPath.empty() || mPath[0] == _T('\0');
}

Path::operator LPCTSTR() const
{
	return cdata();
}

Path::operator LPTSTR()
{
	return data();
}

size_t Path::size() const
{
	return mPath.size();
}

bool Path::Append(LPCTSTR path)
{
	if (PathAppend(data(), path) == FALSE) {
		return false;
	}
	std::replace(mPath.begin(), mPath.end(), _T('/'), _T('\\'));
	return true;
}

bool Path::AddExtension(LPCTSTR ext)
{
	return PathAddExtension(data(), ext) != FALSE;
}

LPCTSTR Path::FindFileName() const
{
	return PathFindFileName(cdata());
}

LPCTSTR Path::FindExtension() const
{
	return PathFindExtension(cdata());
}

bool Path::RemoveFileSpec()
{
	return PathRemoveFileSpec(data()) != FALSE;
}

void Path::RemoveExtension()
{
	PathRemoveExtension(data());
}

bool Path::RenameExtension(LPCTSTR ext)
{
	return PathRenameExtension(data(), ext) != false;
}

bool Path::FileExists() const
{
	return Path::FileExists(cdata());
}

bool Path::IsDirectory() const
{
	return Path::IsDirectory(cdata());
}

bool Path::IsURL() const
{
	return PathIsURL(cdata()) != FALSE;
}

bool Path::IsRelative() const
{
	return PathIsRelative(cdata()) != FALSE;
}

HRESULT Path::CreateFromUrl(LPCTSTR url, DWORD flags)
{
	DWORD len = (DWORD)size();
	return PathCreateFromUrl(url, data(), &len, flags);
}

// 実際のパスの長さにbufferを切り詰める
void Path::Shrink()
{
	size_t actualLen = _tcslen(cdata());
	mPath.resize(actualLen + 1);
	mPath.shrink_to_fit();
}

LPCTSTR Path::cdata() const
{
	return mPath.data();
}
LPTSTR Path::data()
{
	return mPath.data();
}

bool Path::FileExists(LPCWSTR pathStr)
{
	if (PathIsUNCW(pathStr) == FALSE) {
		// UNC形式でなければ単にAPIをよぶ
		return PathFileExistsW(pathStr) != FALSE;
	}
	else {
		// UNC形式で無効化パスの場合、しばらく応答が返らなくなるので、バックグラウンドで実行してタイムアウト処理を入れる
		std::wstring path(pathStr);

		auto queryData = QueryDataHost::Get()->RequestFileExists(path);
		if (queryData == nullptr) {
			return false;
		}

		bool result = queryData->TryGet(100);
		queryData->Release();

		return result;
	}
}

bool Path::FileExists(LPCSTR pathStr)
{
	if (PathIsUNCA(pathStr) == FALSE) {
		// UNC形式でなければ単にAPIをよぶ
		return PathFileExistsA(pathStr) != FALSE;
	}
	else {
		// UNC形式で無効化パスの場合、しばらく応答が返らなくなるので、バックグラウンドで実行してタイムアウト処理を入れる
		std::wstring path;
		UTF2UTF(pathStr, path);

		auto queryData = QueryDataHost::Get()->RequestFileExists(path);
		if (queryData == nullptr) {
			return false;
		}

		bool result = queryData->TryGet(100);
		queryData->Release();

		return result;
	}
}

bool Path::IsDirectory(LPCTSTR pathStr)
{
	if (PathIsUNC(pathStr) == FALSE) {
		// UNC形式でなければ単にAPIをよぶ
		return PathIsDirectory(pathStr) != FALSE;
	}
	else {
		// UNC形式で無効化パスの場合、しばらく応答が返らなくなるので、バックグラウンドで実行してタイムアウト処理を入れる
		tstring path(pathStr);

		auto queryData = QueryDataHost::Get()->RequestIsDirectory(path);
		if (queryData == nullptr) {
			return false;
		}

		bool result = queryData->TryGet(100);
		queryData->Release();

		return result;
	}
}

// 指定したホストに対する接続が確立しているかを確認する
bool Path::IsHostConnected(const CString& targetHost)
{
	LPBYTE buffer = nullptr;
	DWORD entriesRead = 0;
	DWORD totalEntries = 0;
	DWORD resumeHandle = 0;

	// ローカル コンピューターとリモート サーバー上のリソース間のすべての現在の接続を一覧を取得
	NET_API_STATUS status = NetUseEnum(nullptr, 2, &buffer, MAX_PREFERRED_LENGTH,
	                                   &entriesRead, &totalEntries, &resumeHandle);
	if (status != NERR_Success || buffer == nullptr) {
		return false;
	}

	struct scope_free {
		scope_free(LPBYTE buf) : mBuffer(buf) {}
		~scope_free() { NetApiBufferFree(mBuffer); }
		LPBYTE mBuffer{nullptr};
	} _scope(buffer);

	USE_INFO_2* useInfo = (USE_INFO_2*)buffer;

	for (DWORD i = 0; i < entriesRead; ++i) {

		// UNCパスかどうかを判断
		// 例: remoteName = "\\\\server\\share"
		std::wstring remoteName(useInfo[i].ui2_remote ? useInfo[i].ui2_remote : L"");
		if (remoteName.find(L"\\\\") != 0) {
			continue;
		}

		// ホスト名を取り出す
		size_t end = remoteName.find(L'\\', 2);
		std::wstring host = remoteName.substr(0, end);

		// ホスト名が所望のものかを判断する
		if (_wcsicmp(host.c_str(), targetHost) != 0) {
			continue;
		}

		// 発見
		return true;
	}

	return false;
}
