#include "pch.h"
#include "ShellDestinationCommandProvider.h"
#include "AutomaticDestinationsFile.h"
#include "ShellDestinationCommand.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "utility/LocalDirectoryWatcher.h"
#include "utility/Path.h"
#include "utility/RefPtr.h"

#include <mutex>

#pragma warning(push)
#pragma warning(disable: 4995)
#pragma warning(disable: 4324)
#include <absl/container/btree_map.h>
#pragma warning(pop)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace shell_destinations {

namespace {

constexpr ULONGLONG FILE_TIME_TICKS_PER_DAY = 24ULL * 60 * 60 * 10000000;
constexpr ULONGLONG MAX_FILE_AGE = 90ULL * FILE_TIME_TICKS_PER_DAY;
constexpr DWORD FILE_RELOAD_DELAY_MSEC = 250;

using EntryMap = absl::btree_map<std::wstring, std::vector<AutomaticDestinationEntry>>;

/**
  更新日時が90日以内のAutomaticDestinationsファイルを読み込む
  @param[in] directory 対象ディレクトリ
  @param[out] entries アプリIDごとにまとめた項目一覧
*/
void LoadDestinationEntries(const CString& directory, EntryMap& entries)
{
	FILETIME currentTime{};
	GetSystemTimeAsFileTime(&currentTime);
	ULARGE_INTEGER now{};
	now.LowPart = currentTime.dwLowDateTime;
	now.HighPart = currentTime.dwHighDateTime;

	CString searchPath(directory);
	if (searchPath.Right(1) != _T("\\")) {
		searchPath += _T("\\");
	}
	searchPath += _T("*.automaticdestinations-ms");

	WIN32_FIND_DATA findData{};
	HANDLE findHandle = FindFirstFile(searchPath, &findData);
	if (findHandle == INVALID_HANDLE_VALUE) {
		return;
	}

	do {
		if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			continue;
		}

		ULARGE_INTEGER modified{};
		modified.LowPart = findData.ftLastWriteTime.dwLowDateTime;
		modified.HighPart = findData.ftLastWriteTime.dwHighDateTime;
		if (modified.QuadPart > now.QuadPart || now.QuadPart - modified.QuadPart > MAX_FILE_AGE) {
			continue;
		}

		CString filePath(directory);
		if (filePath.Right(1) != _T("\\")) {
			filePath += _T("\\");
		}
		filePath += findData.cFileName;

		AutomaticDestinationsFile destinationFile;
		if (destinationFile.Open(filePath) == false) {
			continue;
		}

		CString fileName(findData.cFileName);
		int extensionPos = fileName.ReverseFind(_T('.'));
		if (extensionPos <= 0) {
			continue;
		}
		fileName = fileName.Left(extensionPos);

		auto& appEntries = entries[std::wstring(fileName)];
		for (size_t i = 0; i < destinationFile.GetPinnedEntryCount(); ++i) {
			AutomaticDestinationEntry entry{};
			if (destinationFile.GetPinnedEntry(i, entry)) {
				appEntries.push_back(std::move(entry));
			}
		}
	} while (FindNextFile(findHandle, &findData));

	FindClose(findHandle);
}

} // 無名名前空間

struct ShellDestinationCommandProvider::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this, _T("ShellDestinationCommandProvider"));
	}
	~PImpl() override
	{
		AppPreference::Get()->UnregisterListener(this);
		Stop();
	}

	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		SetEnable(AppPreference::Get()->IsUseShellDestinations());
	}
	void OnAppExit() override {}

	void SetEnable(bool enable)
	{
		{
			std::lock_guard<std::mutex> lock(mMutex);
			if (mIsEnable == enable) {
				return;
			}
			mIsEnable = enable;
		}

		if (enable) {
			Start();
		}
		else {
			Stop();
		}
	}

	void Start()
	{
		Path directory(Path::APPDATA, _T("Microsoft\\Windows\\Recent\\AutomaticDestinations"));
		if (Path::IsDirectory(directory)) {
			auto id = LocalDirectoryWatcher::GetInstance()->Register(directory,
				[](void* param) {
					auto self = static_cast<PImpl*>(param);
					Sleep(FILE_RELOAD_DELAY_MSEC);
					self->Reload();
				}, this);
			bool shouldUnregister = false;
			{
				std::lock_guard<std::mutex> lock(mMutex);
				if (mIsEnable) {
					mWatchId = id;
				}
				else {
					shouldUnregister = id != 0;
				}
			}
			if (shouldUnregister) {
				LocalDirectoryWatcher::GetInstance()->Unregister(id);
			}
		}
		Reload();
	}

	void Stop()
	{
		uint32_t watchId = 0;
		{
			std::lock_guard<std::mutex> lock(mMutex);
			watchId = mWatchId;
			mWatchId = 0;
			mEntries.clear();
		}
		if (watchId != 0) {
			LocalDirectoryWatcher::GetInstance()->Unregister(watchId);
		}
	}

	void Reload()
	{
		{
			std::lock_guard<std::mutex> lock(mMutex);
			if (mIsEnable == false) {
				return;
			}
		}

		Path directory(Path::APPDATA, _T("Microsoft\\Windows\\Recent\\AutomaticDestinations"));
		EntryMap entries;
		LoadDestinationEntries(CString((LPCTSTR)directory), entries);

		std::lock_guard<std::mutex> lock(mMutex);
		if (mIsEnable) {
			mEntries.swap(entries);
		}
	}

	std::mutex mMutex;
	bool mIsEnable{false};
	uint32_t mWatchId{0};
	EntryMap mEntries;
};

REGISTER_COMMANDPROVIDER(ShellDestinationCommandProvider)

ShellDestinationCommandProvider::ShellDestinationCommandProvider() : in(std::make_unique<PImpl>())
{
}

ShellDestinationCommandProvider::~ShellDestinationCommandProvider() = default;

CString ShellDestinationCommandProvider::GetName()
{
	return _T("ShellDestinationCommand");
}

void ShellDestinationCommandProvider::PrepareAdhocCommands()
{
	in->SetEnable(AppPreference::Get()->IsUseShellDestinations());
}

void ShellDestinationCommandProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands)
{
  if (in->mIsEnable == false) {
    return;
  }

  std::lock_guard<std::mutex> lock(in->mMutex);

  CString name;

  for (const auto& app : in->mEntries) {

    for (const auto& entry : app.second) {

      name = entry.displayPath.c_str();
      auto separatorPos = name.ReverseFind(_T('\\'));
      int slashPos = name.ReverseFind(_T('/'));
      if (slashPos > separatorPos) {
        separatorPos = slashPos;
      }
      if (separatorPos >= 0) {
        name = name.Mid(separatorPos + 1);
      }

      int queryPos = name.Find(_T('?'));
      if (queryPos >= 0) {
        name = name.Left(queryPos);
      }
      int fragmentPos = name.Find(_T('#'));
      if (fragmentPos >= 0) {
        name = name.Left(fragmentPos);
      }

      int level = pattern->Match(name);
      if (level == Pattern::Mismatch) {
        continue;
      }
      auto command = make_refptr<ShellDestinationCommand>(entry);
      commands.Add(CommandQueryItem(level, command.release()));
    }
  }
}

uint32_t ShellDestinationCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(_T("ピン留めされた項目"));
	return 1;
}

}}}
