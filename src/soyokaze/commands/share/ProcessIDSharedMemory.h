 #pragma once

// サーバ(子プロセス)によって、通常権限で起動したコマンドプロセスのPIDを、
// 管理者特権で動作する親プロセス側に渡すための共有メモリクラス
class ProcessIDSharedMemory
{
	static constexpr int NumOfArrays = 16;
public:
	ProcessIDSharedMemory()
	{
		// 共有メモリ領域を作成する
		mMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(DWORD) * NumOfArrays, _T("LauncherAppProcessIDShare"));
		if (mMapFile != nullptr) {
			mPID = (DWORD*) MapViewOfFile(mMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DWORD) * NumOfArrays);
		}
	}
	~ProcessIDSharedMemory()
	{
		// 共有メモリ領域を破棄する
		if (mPID) {
			UnmapViewOfFile(mPID);
		}
		if (mMapFile) {
			CloseHandle(mMapFile);
		}
	}

	int IssueIndex() {
		int indexPID = mNextIndex++;
		if (mNextIndex >= NumOfArrays) {
			mNextIndex = 0;
		}
		return indexPID;
	}

	DWORD GetPID(int indexPID)
	{
		return mPID ? mPID[indexPID] : 0xFFFFFFFF;
	}

	/**
	 	起動したPIDを共有メモリに書き込む
		launcher_proxy.exe側が使う処理
	 	@param[in] indexPID 書き込む位置
	 	@param[in] pid      PID
	*/
	static void RegisterPID(int indexPID, DWORD pid)
	{
		// 親プロセス側がひらいた共有メモリ領域を開く
		HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("LauncherAppProcessIDShare"));
		if (hMapFile == nullptr) {
			return;
		}
		auto pids = (DWORD*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DWORD) * NumOfArrays);
		if (pids == nullptr) {
			return;
		}

		pids[indexPID] = pid;

		UnmapViewOfFile(pids);
		CloseHandle(hMapFile);
	}

private:
	HANDLE mMapFile = nullptr;
	DWORD* mPID = nullptr;
	int mNextIndex = 0;
};



