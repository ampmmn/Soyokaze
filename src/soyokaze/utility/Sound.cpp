#include "pch.h"
#include "Sound.h"
#include "SharedHwnd.h"
#include "utility/Path.h"
#include "utility/MessageExchangeWindow.h"
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
	bool GetWaitingItem(const CString& filePath, ItemPtr& item);
	void OnPlaySoundCompleted(MCIDEVICEID deviceID);

	MessageExchangeWindow mNotifyHwnd;

	ItemList mPlayingItems;
	std::map<CString, ItemList> mWaitingMap;

	int mAliasIndex{1};
};

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

void Sound::PImpl::OnPlaySoundCompleted(MCIDEVICEID deviceID)
{
	// 再生中のデバイスIDを先頭に戻し、待機リストに戻す
	for (auto it = mPlayingItems.begin(); it != mPlayingItems.end(); ++it) {
		auto& item = *it;
		if (deviceID != item->mParam.wDeviceID) {
			continue;
		}

		mciSendCommand(deviceID, MCI_SEEK, MCI_SEEK_TO_START, 0);

		// 待機リストに追加
		mWaitingMap[item->mFilePath].push_back(std::move(item));

		// 再生中リストから除外
		mPlayingItems.erase(it);
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



Sound::Sound() : in(std::make_unique<PImpl>())
{
	// サウンドの再生が完了した通知を受け取り、処理するためのコールバック関数
	in->mNotifyHwnd.SetCallback([&](HWND h, UINT msg, WPARAM wp, LPARAM lp) -> LRESULT {
		if (msg != MM_MCINOTIFY || wp != MCI_NOTIFY_SUCCESSFUL) {
			return DefWindowProc(h, msg, wp, lp);
		}
		in->OnPlaySoundCompleted((MCIDEVICEID)lp);
		return 0;
	});
}

Sound::~Sound()
{
	for (auto it = in->mWaitingMap.begin(); it != in->mWaitingMap.end(); ++it) {
		for (auto& item : it->second) {
			mciSendCommand(item->mParam.wDeviceID, MCI_CLOSE, 0, 0);
		}
	}
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
	if (in->mNotifyHwnd.Exists() == false) {
		in->mNotifyHwnd.Create();
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
	playParam.dwCallback= (DWORD_PTR)in->mNotifyHwnd.GetHwnd();
	mciSendCommand(playItem->mParam.wDeviceID, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&playParam);

	//size_t waitingCount = in->mWaitingMap[playItem->mFilePath].size();
	//TRACE(_T("Sound: deviceid %d play.\n"), playItem->mParam.wDeviceID);
	//TRACE(_T("Sound: playing/waiting %d/%d\n"), (int)in->mPlayingItems.size(), (int)waitingCount);

	// 再生中アイテムとして登録
	in->mPlayingItems.push_back(std::move(playItem));

	return true;
}

} // end of namespace utility

