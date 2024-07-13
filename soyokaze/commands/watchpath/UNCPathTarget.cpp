#include "pch.h"
#include "UNCPathTarget.h"
#include <vector>
#include <deque>
#include <map>
#include "utility/SHA1.h"
#include "spdlog/stopwatch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace watchpath {

using TimeStampList = std::vector<std::pair<CString, DWORD> >;

struct DirectoryNode
{
	struct CompareName
	{
		bool operator() (const CString& l, const CString& r) const
		{
			return l.CompareNoCase(r) < 0;
		}
	};

	DirectoryNode()
	{
	}
	~DirectoryNode()
	{
		Clear();
	}
	void Clear()
	{
		for(auto& item : mChildren) {
			delete item.second;
		}
	}

	bool Build(const CString& path)
	{
		// 対象パスが存在しない場合
		if (PathIsDirectory(path) == FALSE) {
			return false;
		}

		spdlog::stopwatch sw;

		Clear();

		mName = path;

		using Pair = std::pair<DirectoryNode*,CString>;

		// フォルダ内のファイル列挙
		std::deque<Pair> stk;
		stk.push_back(Pair(this, path));

		while(stk.empty() == false) {

			auto& elem = stk.front();

			DirectoryNode* parent = elem.first;
			CString curDir = elem.second;
			stk.pop_front();

			PathAppend(curDir.GetBuffer(MAX_PATH_NTFS), _T("*.*"));
			curDir.ReleaseBuffer();

			CFileFind f;
			BOOL isLoop = f.FindFile(curDir, 0);
			while (isLoop) {

				Sleep(0);

				isLoop = f.FindNextFile();

				if (f.IsDots()) {
					continue;
				}

				if (f.IsDirectory()) {

					auto subDirPath = f.GetFilePath();

					DirectoryNode* subDirNode = new DirectoryNode();
					subDirNode->mName = PathFindFileName(subDirPath);
					parent->mChildren[PathFindFileName(subDirPath)] = subDirNode;

					stk.push_back(Pair(subDirNode, subDirPath));
					continue;
				}

				CString filePath = f.GetFilePath();

				FILETIME ft;
				f.GetLastWriteTime(&ft);

				parent->mFiles[PathFindFileName(filePath)] = ft.dwLowDateTime;
			}
			f.Close();
		}

		SPDLOG_DEBUG("Elapsed : {:.6f} s", sw);

		return true;
	}

	bool GetLastChangedItem(CString& path, int& type)
	{
		path = mLastChangedItem;
		type = mLastChangedAction;
		return true;
	}

	void swap(DirectoryNode& rhs)
	{
		std::swap(mName, rhs.mName);
		mFiles.swap(rhs.mFiles);
		mChildren.swap(rhs.mChildren);
	}

	bool IsDiffer(const DirectoryNode& other)
	{
		return IsDifferInternal(other, mName, mLastChangedItem, mLastChangedAction);
	}

private:

	bool IsDifferInternal(const DirectoryNode& other, LPCTSTR basePath, CString& lastChangedItem, int& action)
	{
		if (mName.CompareNoCase(other.mName) != 0) {
			return true;
		}

		for (auto& item : mFiles) {
			auto& name = item.first;
			auto it = other.mFiles.find(name);
			if (it == other.mFiles.end()) {
				MakePath(lastChangedItem, basePath, name);
				action = 0;  // added
				return true;
			}
			if (memcmp(&item.second, &(it->second), sizeof(DWORD)) != 0) {
				MakePath(lastChangedItem, basePath, name);
				action = 2;  // updated
				return true;
			}
		}
		for (auto& item : other.mFiles) {
			auto& name = item.first;
			auto it = mFiles.find(name);
			if (it != mFiles.end()) {
				continue;
			}
			MakePath(lastChangedItem, basePath, name);
			action = 1; // deleted
			return true;
		}

		for (auto& item : mChildren) {
			auto& name = item.first;
			auto it = other.mChildren.find(name);
			if (it == other.mChildren.end()) {
				MakePath(lastChangedItem, basePath, name);
				action = 0;  // added
				return true;
			}

			TCHAR fullPath[MAX_PATH_NTFS];
			_tcscpy_s(fullPath, basePath);
			PathAppend(fullPath, name);

			if (item.second->IsDifferInternal(*it->second, fullPath, lastChangedItem, action)) {
				return true;
			}
		}

		for (auto& item : other.mChildren) {
			auto& name = item.first;
			auto it = mChildren.find(name);
			if (it != mChildren.end()) {
				continue;
			}
			MakePath(lastChangedItem, basePath, name);
			action = 1; // deleted
			return true;
		}

		return false;
	}

	void MakePath(CString& path, LPCTSTR baseDir, const CString& itemName)
	{
		path = baseDir;
		PathAppend(path.GetBuffer(MAX_PATH_NTFS), itemName);
		path.ReleaseBuffer();
	}

public:
	CString mName;
	std::map<CString, DWORD, CompareName> mFiles;
	std::map<CString, DirectoryNode*, CompareName> mChildren;
	CString mLastChangedItem;
	int mLastChangedAction = 0;
};



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct UNCPathTarget::PImpl
{
	CString mCommandName;
	CString mPath;
	CString mMessage;
	CString mDetail;
	// 監視対象パス以下にある要素の更新日時情報
	DirectoryNode mPrevTimeStamps;
	// 最後にチェックした時刻
	DWORD mLastCheckTime = 0;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////




UNCPathTarget::UNCPathTarget(
	const CString& cmdName,
 	const CString& message,
 	const CString& path
) : in(new PImpl)
{
	in->mCommandName = cmdName;
	in->mMessage = message;
	in->mPath = path;
}

UNCPathTarget::~UNCPathTarget()
{
}

bool UNCPathTarget::IsUpdated()
{
	//bool isFirst = in->mLastCheckTime == 0;
	//if (isFirst == false) {
	//	DWORD elapsed = GetTickCount() - in->mLastCheckTime;
	//	if (elapsed < 5000) {  // FIXME: 間隔調整
	//		return false;
	//	}
	//}

	// 現在の情報を取得する
	DirectoryNode currentTS;
	if (currentTS.Build(in->mPath) == false) {
		return false;
	}

	// 初回だった場合は比較しない
	if (in->mLastCheckTime == 0) {
		in->mLastCheckTime = GetTickCount();
		in->mPrevTimeStamps.swap(currentTS);
		return false;
	}

	// 前回と比較
	if (currentTS.IsDiffer(in->mPrevTimeStamps) == false) {
		in->mLastCheckTime = GetTickCount();
		return false;
	}

	int action = 0;
	CString changedItem;
	currentTS.GetLastChangedItem(changedItem, action);

	CString typeStr;
	switch(action) {
	case 0:
 		typeStr = _T("[追加]");
		break;
	case 1:
 		typeStr = _T("[削除]");
		break;
	case 2:
	default:
 		typeStr = _T("[変更]");
		break;
	}

	in->mDetail.Format(_T("%s %s"), (LPCTSTR)typeStr, (LPCTSTR)changedItem);
	in->mLastCheckTime = GetTickCount();
	in->mPrevTimeStamps.swap(currentTS);
	return true;
}

CString UNCPathTarget::GetCommandName()
{
	return in->mCommandName;
}

CString UNCPathTarget::GetTitle()
{
	return in->mMessage;
}

CString UNCPathTarget::GetDetail()
{
	return in->mDetail;
}


}
}
}

