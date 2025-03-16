 #pragma once

// $B%5!<%P(B($B;R%W%m%;%9(B)$B$K$h$C$F!"DL>o8"8B$G5/F0$7$?%3%^%s%I%W%m%;%9$N(BPID$B$r!"(B
// $B4IM}<TFC8"$GF0:n$9$k?F%W%m%;%9B&$KEO$9$?$a$N6&M-%a%b%j%/%i%9(B
class ProcessIDSharedMemory
{
	static constexpr int NumOfArrays = 16;
public:
	ProcessIDSharedMemory()
	{
		// $B6&M-%a%b%jNN0h$r:n@.$9$k(B
		mMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(DWORD) * NumOfArrays, _T("LauncherAppProcessIDShare"));
		if (mMapFile != nullptr) {
			mPID = (DWORD*) MapViewOfFile(mMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DWORD) * NumOfArrays);
		}
	}
	~ProcessIDSharedMemory()
	{
		// $B6&M-%a%b%jNN0h$rGK4~$9$k(B
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
	 	$B5/F0$7$?(BPID$B$r6&M-%a%b%j$K=q$-9~$`(B
		launcher_proxy.exe$BB&$,;H$&=hM}(B
	 	@param[in] indexPID $B=q$-9~$`0LCV(B
	 	@param[in] pid      PID
	*/
	static void RegisterPID(int indexPID, DWORD pid)
	{
		// $B?F%W%m%;%9B&$,$R$i$$$?6&M-%a%b%jNN0h$r3+$/(B
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



