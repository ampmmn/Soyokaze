#include "pch.h"
#include "ExplorePathFileCache.h"

#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace explorepath {

namespace {

// 大きなフォルダはキャッシュせず、列挙中の要素だけを呼び出し元へ渡す。
constexpr size_t MAX_CACHE_ENTRIES = 4096;

CString NormalizeFolderPath(const CString& path)
{
	CString normalized(path);
	normalized.Replace(_T('/'), _T('\\'));

	// ドライブのルート("C:\\")は末尾の区切り文字を残す。
	while (normalized.GetLength() > 3 && normalized.Right(1) == _T("\\")) {
		normalized.Delete(normalized.GetLength() - 1);
	}
	return normalized;
}

} // namespace

struct ExplorePathFileCache::PImpl
{
	void Clear()
	{
		mFolderPath.Empty();
		mEntries.clear();
		mHasCache = false;
		mIsCacheable = false;
	}

	bool IsSameFolder(const CString& folderPath) const
	{
		return mHasCache && mFolderPath.CompareNoCase(folderPath) == 0;
	}

	void Reset(const CString& folderPath)
	{
		mFolderPath = folderPath;
		mEntries.clear();
		mHasCache = true;
		mIsCacheable = true;
	}

	CString mFolderPath;
	std::vector<Entry> mEntries;
	bool mHasCache{false};
	bool mIsCacheable{false};
};

ExplorePathFileCache::ExplorePathFileCache() : in(std::make_unique<PImpl>())
{
}

ExplorePathFileCache::~ExplorePathFileCache()
{
}

bool ExplorePathFileCache::ForEach(
	const CString& folderPath,
	const EntryCallback& callback
)
{
	CString normalizedPath = NormalizeFolderPath(folderPath);
	if (in->IsSameFolder(normalizedPath) && in->mIsCacheable) {
		for (const auto& entry : in->mEntries) {
			callback(entry);
		}
		return true;
	}

	if (in->IsSameFolder(normalizedPath) == false) {
		in->Reset(normalizedPath);
	}

	CString findPattern(normalizedPath + _T("\\*.*"));
	WIN32_FIND_DATA findData{};
	HANDLE findHandle = FindFirstFileEx(
		findPattern,
		FindExInfoBasic,
		&findData,
		FindExSearchNameMatch,
		nullptr,
		FIND_FIRST_EX_LARGE_FETCH
	);
	if (findHandle == INVALID_HANDLE_VALUE) {
		// 大容量列挙がサポートされないファイルシステムでは通常の列挙に戻す。
		findHandle = FindFirstFileEx(
			findPattern,
			FindExInfoStandard,
			&findData,
			FindExSearchNameMatch,
			nullptr,
			0
		);
	}
	if (findHandle == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			// 空のフォルダも有効なキャッシュとして扱う。
			in->mIsCacheable = true;
			in->mEntries.clear();
			return true;
		}
		in->Clear();
		return false;
	}

	bool isCacheable = true;
	for (;;) {
		if (_tcscmp(findData.cFileName, _T(".")) != 0 &&
			_tcscmp(findData.cFileName, _T("..")) != 0) {
			Entry entry;
			entry.mName = findData.cFileName;
			entry.mIsDirectory =
				(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

			if (isCacheable) {
				if (in->mEntries.size() >= MAX_CACHE_ENTRIES) {
					// 途中までの結果をキャッシュすると後続のキーワード検索で
					// 候補漏れが発生するため、超過時は全体を破棄する。
					in->mEntries.clear();
					isCacheable = false;
				}
				else {
					in->mEntries.push_back(entry);
				}
			}

			callback(entry);
		}

		if (FindNextFile(findHandle, &findData) == FALSE) {
			break;
		}
	}
	FindClose(findHandle);

	in->mIsCacheable = isCacheable;
	if (isCacheable == false) {
		in->mEntries.clear();
	}
	return true;
}

void ExplorePathFileCache::Clear()
{
	in->Clear();
}

}}} // end of namespace launcherapp::commands::explorepath
