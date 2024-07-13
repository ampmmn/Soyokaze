#include "pch.h"
#include "LocalPathTarget.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace watchpath {

struct LocalPathTarget::PImpl
{
	// インスタンスが持つ各種ハンドルをクリアする
	void Reset()
	{
		if (mDirHandle) {

			CancelIo(mDirHandle);
			WaitForSingleObject(mOverlapped.hEvent, 500);

			CloseHandle(mDirHandle);
			mDirHandle = nullptr;
		}
		if (mOverlapped.hEvent) {
			CloseHandle(mOverlapped.hEvent);
			mOverlapped.hEvent = nullptr;
		}
	}

	// 更新検知用のハンドルを作成する
	bool CreateHandle()
	{
		// CreateFileで監視対象(フォルダ)のhandleを生成する
		HANDLE h = CreateFile(mPath, FILE_LIST_DIRECTORY,
		                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				                  NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		if (h == INVALID_HANDLE_VALUE) {
			return false;
		}

		// 変更通知受信用のバッファを作成する
		mBuffer.resize(sizeof(FILE_NOTIFY_INFORMATION) + sizeof(TCHAR) * MAX_PATH_NTFS);

		// 更新検知するためのイベントを作成する
		HANDLE hEventAuto = CreateEvent(NULL, TRUE, FALSE, NULL);
		mOverlapped.hEvent = hEventAuto;
		mDirHandle = h;

		// ReadDirectoryChangesWをコールして監視を開始する
		if (CallReadDirectoryChanges() == FALSE) {
			mDirHandle = nullptr;
			mOverlapped.hEvent = nullptr;
			CloseHandle(hEventAuto);
			CloseHandle(h);
			return false;
		}		
		return true;
	}

	bool CallReadDirectoryChanges()
	{
		// 変更検知の対象とする対象のビットフラグ
		DWORD flags = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
		             	FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE;

		ResetEvent(mOverlapped.hEvent);
		BOOL isOK = ReadDirectoryChangesW(mDirHandle, &mBuffer.front(), (DWORD)mBuffer.size(), TRUE,
		                                  flags, nullptr, &mOverlapped, nullptr);
		return isOK;
	}

	// 
	bool PrepareInformation()
	{
		DWORD size = 0;
		BOOL isOK = GetOverlappedResult(mDirHandle, &mOverlapped, &size, FALSE);
		ResetEvent(mOverlapped.hEvent);
		if (isOK == FALSE) {
			return false;
		}

		// 変更通知情報を受け取る
		FILE_NOTIFY_INFORMATION* data = (FILE_NOTIFY_INFORMATION*)&mBuffer.front();

		int itemCount = 0;

		// 最初に通知されたものを代表して通知するので保持しておく
		int actionFirst = 0;
		CString filePathFirst;

		std::vector<TCHAR> fileName;
		for (;;) {

			int action = data->Action;
				// FILE_ACTION_ADDED or FILE_ACTION_REMOVED or FILE_ACTION_MODIFIED or
				// FILE_ACTION_RENAMED_OLD_NAME or FILE_ACTION_RENAMED_NEW_NAME

			if (itemCount == 0) {
				// 最初の要素だけ保持しておく
				DWORD lenInByte = data->FileNameLength;
				size_t lenInStrCount = lenInByte / sizeof(TCHAR);

				fileName.resize(lenInStrCount + 1);
				memcpy(&fileName.front(), data->FileName, lenInByte);
				fileName[lenInStrCount] = _T('\0');
				actionFirst = action;
				filePathFirst = &fileName.front();
			}
			itemCount++;

			if (data->NextEntryOffset == 0) {
				break;
			}
			data = (FILE_NOTIFY_INFORMATION*)((LPBYTE)(data) + data->NextEntryOffset);
		}

		// 更新されたファイルパス情報をもとに通知用の文字列を生成する
		MakeDetailMessage(actionFirst, filePathFirst, itemCount);

		// フォルダがなくなっていたら終了
		if (PathIsDirectory(mPath) == FALSE) {
			Reset();
			return true;
		}

		// 再度ReadDirectoryChanges
		CallReadDirectoryChanges();

		return true;
	}

	void MakeDetailMessage(int action, const CString& path, int itemCount)
	{
		CString act;
		switch(action) {
			case FILE_ACTION_ADDED:
				act = _T("[追加]");
				break;
			case FILE_ACTION_REMOVED:
				act = _T("[削除]");
				break;
			case FILE_ACTION_MODIFIED:
				act = _T("[更新]");
				break;
			case FILE_ACTION_RENAMED_OLD_NAME:
			case FILE_ACTION_RENAMED_NEW_NAME:
				act = _T("[ファイル名変更]");
				break;
		}

		mDetail.Format(_T("%s %s"), (LPCTSTR)act, (LPCTSTR)path);

		if (itemCount > 1) {
			CString countMsg;
			countMsg.Format(_T(" ほか %dつ"), itemCount-1);
			mDetail += countMsg;
		}
	}


	// コマンド名
	CString mCommandName;
	// 監視対象パス
	CString mPath;
	// 検知時のメッセージ
	CString mMessage;
	//
	CString mDetail;

	// ReadDirectoryChangesWに渡すためのファイルハンドル
	HANDLE mDirHandle = nullptr;
	// 更新検知を受け取るためのイベントを含むOVERLAPPED構造体
	OVERLAPPED mOverlapped = {};
	// 変更通知受信用のバッファ
	std::vector<BYTE> mBuffer;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



LocalPathTarget::LocalPathTarget(
	const CString& cmdName,
 	const CString& message,
 	const CString& path
) : in(new PImpl)
{
	in->mCommandName = cmdName;
	in->mMessage = message;
	in->mPath = path;
}

LocalPathTarget::~LocalPathTarget()
{
	in->Reset();
}

bool LocalPathTarget::IsUpdated()
{
	// 監視対象の有無を確認
	if (PathIsDirectory(in->mPath) == FALSE) {
		in->Reset();
		return false;
	}

	// 初回呼び出し時に初期化を行う
	if (in->mDirHandle == nullptr && in->CreateHandle() == false) {
		return false;
	}
	ASSERT(in->mOverlapped.hEvent);

	// 更新があったか確認する
	if (WaitForSingleObject(in->mOverlapped.hEvent, 0) == WAIT_TIMEOUT) {
		return false;
	}

	// 変更通知情報を取得する
	if (in->PrepareInformation() == false) {
		return false;
	}

	return true;  // 更新あり
}

CString LocalPathTarget::GetCommandName()
{
	return in->mCommandName;
}

CString LocalPathTarget::GetTitle()
{
	return in->mMessage;
}

CString LocalPathTarget::GetDetail()
{
	return in->mDetail;
}

}
}
}

