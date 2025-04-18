#include "pch.h"
#include "Sound.h"
#include "SharedHwnd.h"
#include "utility/Path.h"
#include <mmsystem.h>
#include <list>
#include <map>

#pragma comment(lib, "winmm.lib")

namespace utility {

static LPCTSTR DEVICETYPE = _T("MPEGVideo");

struct Sound::ITEM
{
	MCI_OPEN_PARMS mParam{};
	CString mFilePath;
	CString mAlias;
};

using ItemPtr = std::unique_ptr<Sound::ITEM>;

using ItemList = std::list<ItemPtr>;

struct Sound::PImpl
{
	bool IsInitialized();
	void Initialize();
	bool GetWaitingItem(const CString& filePath, ItemPtr& item);

	HWND mNotifyHwnd{nullptr};

	ItemList mPlayingItems;
	std::map<CString, ItemList> mWaitingMap;

	int mAliasIndex{1};
};

bool Sound::PImpl::IsInitialized()
{
	return IsWindow(mNotifyHwnd);
}

void Sound::PImpl::Initialize()
{
	SharedHwnd sharedWnd;
	HWND hParent = sharedWnd.GetHwnd();
	if (IsWindow(hParent) == FALSE) {
		return;
	}

	HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hParent, GWLP_HINSTANCE);

	mNotifyHwnd = CreateWindow(_T("Static"), _T(""), WS_CHILD, 0, 0, 0, 0, hParent, NULL, hInst, 0);
	ASSERT(mNotifyHwnd);

	SetWindowLongPtr(mNotifyHwnd, GWLP_WNDPROC, (DWORD_PTR)OnPlayCallbackProc);
}


bool Sound::PImpl::GetWaitingItem(const CString& filePath, ItemPtr& itemRet)
{
	auto it = mWaitingMap.find(filePath);
	if (it == mWaitingMap.end()) {
		return false;
	}

	auto& itemList = it->second;
	if (itemList.empty()) {
		return false;
	}

	auto itItem = itemList.begin();
	itemRet = std::move(*itItem);

	itemList.erase(itItem);

	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



Sound::Sound() : in(std::make_unique<PImpl>())
{
}

Sound::~Sound()
{
	for (auto it = in->mWaitingMap.begin(); it != in->mWaitingMap.end(); ++it) {
		for (auto& item : it->second) {
			mciSendCommand(item->mParam.wDeviceID, MCI_CLOSE, 0, 0);
		}
	}

	if (IsWindow(in->mNotifyHwnd)) {
		DestroyWindow(in->mNotifyHwnd);
	}
}

LRESULT CALLBACK
Sound::OnPlayCallbackProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == MM_MCINOTIFY && wp == MCI_NOTIFY_SUCCESSFUL) {

		auto inst = Sound::Get();

		MCIDEVICEID deviceID = (MCIDEVICEID)lp;

		// 再生中のデバイスIDを先頭に戻し、待機リストに戻す
		auto& playingItems = inst->in->mPlayingItems;
		for (auto it = playingItems.begin(); it != playingItems.end(); ++it) {
			auto& item = *it;
			if (deviceID != item->mParam.wDeviceID) {
				continue;
			}

			mciSendCommand(deviceID, MCI_SEEK, MCI_SEEK_TO_START, 0);

			// 待機リストに追加
			inst->in->mWaitingMap[item->mFilePath].push_back(std::move(item));

			// 再生中リストから除外
			playingItems.erase(it);

			// TRACE(_T("Sound: deviceid %d stopped.\n"), deviceID);
			break;
		}
		return 0;
	}

	return DefWindowProc(h, msg, wp, lp);
}

Sound* Sound::Get()
{
	static Sound inst;
	return &inst;
}

bool Sound::PlayAsync(LPCTSTR filePath)
{
	if (Path::FileExists(filePath) == FALSE) {
		return false;
	}
	if (in->IsInitialized() == false) {
		in->Initialize();
	}

	ItemPtr playItem;
	if (in->GetWaitingItem(filePath, playItem) == false) {

		// 待機中の要素がなければ追加する

		playItem = std::make_unique<ITEM>();
		playItem->mParam.lpstrDeviceType = DEVICETYPE;

		playItem->mFilePath = filePath;
		playItem->mParam.lpstrElementName = playItem->mFilePath;

		DWORD commands = MCI_OPEN_TYPE | MCI_OPEN_ELEMENT;

		MCI_OPEN_PARMS& param = playItem->mParam;
		MCIERROR result = mciSendCommand(0, MCI_OPEN, commands, (DWORD_PTR)&param);
		if (result != 0) {
			// 作成できなかった場合はエイリアスをつけて再試行
			commands |= MCI_OPEN_ALIAS;
			playItem->mAlias.Format(_T("device%d"), in->mAliasIndex++);
			playItem->mParam.lpstrAlias = playItem->mAlias;

			result = mciSendCommand(0, MCI_OPEN, commands, (DWORD_PTR)&param);
			if (result != 0) {
				return false;
			}
		}
	}

	MCI_PLAY_PARMS playParam;
	playParam.dwCallback= (DWORD_PTR)in->mNotifyHwnd;
	mciSendCommand(playItem->mParam.wDeviceID, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&playParam);

	//size_t waitingCount = in->mWaitingMap[playItem->mFilePath].size();
	//TRACE(_T("Sound: deviceid %d play.\n"), playItem->mParam.wDeviceID);
	//TRACE(_T("Sound: playing/waiting %d/%d\n"), (int)in->mPlayingItems.size(), (int)waitingCount);

	// 再生中アイテムとして登録
	in->mPlayingItems.push_back(std::move(playItem));

	return true;
}

} // end of namespace utility

