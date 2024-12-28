#include "pch.h"
#include "EjectVolume.h"
#include <winioctl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace ejectvolume {

constexpr int LOCK_TIMEOUT = 10000;
constexpr int LOCK_RETRIES = 20;

static bool GetVolumeHandle(TCHAR letter, HANDLE& volHandle)
{
	TCHAR rootName[] = { letter, _T(':'), _T('\\'), _T('\0') };
	UINT type = GetDriveType(rootName);
	if (type != DRIVE_REMOVABLE && type !=DRIVE_CDROM) {
	 return false;	
	}

	DWORD flagMask = GENERIC_READ;
	if (type == DRIVE_REMOVABLE) {
		type |= GENERIC_WRITE;
	}

	TCHAR volName[] = { _T('\\'), _T('\\'), _T('.'), _T('\\'), letter, _T(':'), _T('\0') };
	volHandle = CreateFile(volName, flagMask, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	return volHandle != INVALID_HANDLE_VALUE;
}

static bool LockVolume(HANDLE hVolume)
{
	int sleepAmount = LOCK_TIMEOUT / LOCK_RETRIES;
	for (int i = 0; i < LOCK_RETRIES; i++) {
		DWORD retBytes;
		if (DeviceIoControl(hVolume, FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0, &retBytes, nullptr)) {
			return true;
		}

		Sleep(sleepAmount);
	}
	return false;
}

static bool DismountVolume(HANDLE hVolume)
{
	DWORD retBytes;
	return DeviceIoControl(hVolume, FSCTL_DISMOUNT_VOLUME, nullptr, 0, nullptr, 0, &retBytes, nullptr) != FALSE;
}

static bool PreventRemovalOfVolume(HANDLE hVolume)
{
	PREVENT_MEDIA_REMOVAL PMRBuffer;
	PMRBuffer.PreventMediaRemoval = FALSE;

	DWORD retBytes;
	return DeviceIoControl(hVolume, IOCTL_STORAGE_MEDIA_REMOVAL, &PMRBuffer, sizeof(PREVENT_MEDIA_REMOVAL), nullptr, 0, &retBytes, nullptr) != FALSE;
}

static bool AutoEjectVolume(HANDLE hVolume)
{
	DWORD retBytes;
	return DeviceIoControl(hVolume, IOCTL_STORAGE_EJECT_MEDIA, nullptr, 0, nullptr, 0, &retBytes, nullptr) != FALSE;
}

bool EjectVolume(TCHAR cDriveLetter, bool* isAutoEject, bool* isRemoveSafely)
{
	if (isAutoEject) {
	 	*isAutoEject = false;
 	}
	if (isRemoveSafely) {
	 	*isRemoveSafely = false;
 	}

	HANDLE volHandle = nullptr;
	if (GetVolumeHandle(cDriveLetter, volHandle) == false) {
		return false;
	}
	bool fAutoEject = false;
	bool fRemoveSafely = false;
	if (LockVolume(volHandle) == false) {
		// ロックできなかった
		CloseHandle(volHandle);
		return false;
	}
	 
	if (DismountVolume(volHandle) == false) {
		// マウント解除できなかった
		CloseHandle(volHandle);
		return false;
	}

	fRemoveSafely = true;

	if (PreventRemovalOfVolume(volHandle)) {
		if (AutoEjectVolume(volHandle)) {
			// イジェクトできた
			fAutoEject = true;
		}
	}
	CloseHandle(volHandle);

	if (isAutoEject) {
		*isAutoEject = fAutoEject;
	}
	if (isRemoveSafely) {
		*isRemoveSafely = fRemoveSafely;
	}
	return true;
}

} // end of namespace ejectvolume
} // end of namespace commands
} // end of namespace launcherapp

