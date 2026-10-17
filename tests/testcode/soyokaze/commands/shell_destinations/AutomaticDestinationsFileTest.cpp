#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/shell_destinations/AutomaticDestinationsFile.h"

#include <objbase.h>
#include <propvarutil.h>
#include <propsys.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <wrl/client.h>

#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <vector>

using AutomaticDestinationsFile = launcherapp::commands::shell_destinations::AutomaticDestinationsFile;

namespace
{
  struct CoTaskMemDeleter
  {
    void operator()(void* memory) const noexcept
    {
      CoTaskMemFree(memory);
    }
  };

  struct PidlDeleter
  {
    using pointer = PIDLIST_ABSOLUTE;

    void operator()(pointer pidl) const noexcept
    {
      ILFree(pidl);
    }
  };

  class ComApartment
  {
  public:
    ComApartment()
      : mResult(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))
    {
    }

    ~ComApartment()
    {
      if (SUCCEEDED(mResult))
      {
        CoUninitialize();
      }
    }

    bool IsAvailable() const
    {
      return SUCCEEDED(mResult) || mResult == RPC_E_CHANGED_MODE;
    }

  private:
    HRESULT mResult;
  };

  /**
    テスト用バイナリに32ビット値を書き込む
    @param[in,out] bytes 書き込み先のデータ
    @param[in] offset 書き込み位置
    @param[in] value 書き込む値
  */
  void WriteUInt32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value)
  {
    for (std::size_t i = 0; i < sizeof(value); ++i)
    {
      bytes[offset + i] = static_cast<std::uint8_t>((value >> (i * 8)) & 0xff);
    }
  }

  /**
    タイトルと説明を設定したShell Linkのバイナリを作成する
    @return true:成功 false:失敗
    @param[in] title System.Titleに設定する文字列。nullptrの場合は設定しない
    @param[in] description Shell Linkの説明文字列
    @param[out] linkData 作成したShell Linkのデータ
  */
  bool CreateShellLinkData(
    const wchar_t* title,
    const wchar_t* description,
    std::vector<std::uint8_t>& linkData)
  {
    Microsoft::WRL::ComPtr<IShellLinkW> shellLink;
    if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
      IID_PPV_ARGS(shellLink.GetAddressOf()))))
    {
      return false;
    }
    if (FAILED(shellLink->SetPath(L"C:\\test.txt")) ||
      FAILED(shellLink->SetDescription(description)))
    {
      return false;
    }

    if (title != nullptr)
    {
      PROPERTYKEY titleKey{};
      if (FAILED(PSGetPropertyKeyFromName(L"System.Title", &titleKey)))
      {
        return false;
      }

      Microsoft::WRL::ComPtr<IPropertyStore> propertyStore;
      if (FAILED(shellLink.As(&propertyStore)))
      {
        return false;
      }

      PROPVARIANT titleValue{};
      if (FAILED(InitPropVariantFromString(title, &titleValue)))
      {
        return false;
      }
      const HRESULT setResult = propertyStore->SetValue(titleKey, titleValue);
      PropVariantClear(&titleValue);
      if (FAILED(setResult) || FAILED(propertyStore->Commit()))
      {
        return false;
      }
    }

    Microsoft::WRL::ComPtr<IPersistStream> persistStream;
    if (FAILED(shellLink.As(&persistStream)))
    {
      return false;
    }

    Microsoft::WRL::ComPtr<IStream> stream;
    if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, stream.GetAddressOf())) ||
      FAILED(persistStream->Save(stream.Get(), TRUE)))
    {
      return false;
    }

    STATSTG stat{};
    if (FAILED(stream->Stat(&stat, STATFLAG_NONAME)) ||
      stat.cbSize.QuadPart > (std::numeric_limits<std::size_t>::max)())
    {
      return false;
    }

    linkData.resize(static_cast<std::size_t>(stat.cbSize.QuadPart));
    LARGE_INTEGER streamStart{};
    if (FAILED(stream->Seek(streamStart, STREAM_SEEK_SET, nullptr)))
    {
      return false;
    }

    ULONG bytesRead = 0;
    return SUCCEEDED(stream->Read(linkData.data(), static_cast<ULONG>(linkData.size()), &bytesRead)) &&
      bytesRead == linkData.size();
  }

  /**
    テスト用のAutomatic Destinationsファイルを作成する
    @return true:成功 false:失敗
    @param[in] filePath 作成先のパス
    @param[in] destListPath DestListに格納するパス
    @param[in] linkData Shell Linkのデータ
  */
  bool CreateAutomaticDestinationsFile(
    const wchar_t* filePath,
    const std::wstring& destListPath,
    const std::vector<std::uint8_t>& linkData)
  {
    Microsoft::WRL::ComPtr<IStorage> storage;
    if (FAILED(StgCreateDocfile(filePath, STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
      0, storage.GetAddressOf())))
    {
      return false;
    }

    constexpr std::size_t headerSize = 32;
    constexpr std::size_t entryFixedSize = 114;
    std::vector<std::uint8_t> destList(headerSize + entryFixedSize + destListPath.size() * 2, 0);
    WriteUInt32(destList, 0, 1);
    WriteUInt32(destList, 4, 1);
    WriteUInt32(destList, 8, 1);

    const std::size_t entryOffset = headerSize;
    WriteUInt32(destList, entryOffset + 88, 1);
    WriteUInt32(destList, entryOffset + 108, 0);
    destList[entryOffset + 112] = static_cast<std::uint8_t>(destListPath.size() & 0xff);
    destList[entryOffset + 113] = static_cast<std::uint8_t>((destListPath.size() >> 8) & 0xff);
    for (std::size_t i = 0; i < destListPath.size(); ++i)
    {
      const std::uint16_t character = static_cast<std::uint16_t>(destListPath[i]);
      destList[entryOffset + entryFixedSize + i * 2] = static_cast<std::uint8_t>(character & 0xff);
      destList[entryOffset + entryFixedSize + i * 2 + 1] = static_cast<std::uint8_t>(character >> 8);
    }

    Microsoft::WRL::ComPtr<IStream> destListStream;
    if (FAILED(storage->CreateStream(L"DestList", STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
      0, 0, destListStream.GetAddressOf())))
    {
      return false;
    }

    ULONG bytesWritten = 0;
    if (FAILED(destListStream->Write(destList.data(), static_cast<ULONG>(destList.size()), &bytesWritten)) ||
      bytesWritten != destList.size())
    {
      return false;
    }
    destListStream.Reset();

    Microsoft::WRL::ComPtr<IStream> linkStream;
    if (FAILED(storage->CreateStream(L"1", STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
      0, 0, linkStream.GetAddressOf())))
    {
      return false;
    }

    bytesWritten = 0;
    if (FAILED(linkStream->Write(linkData.data(), static_cast<ULONG>(linkData.size()), &bytesWritten)) ||
      bytesWritten != linkData.size())
    {
      return false;
    }
    return SUCCEEDED(storage->Commit(STGC_DEFAULT));
  }

  /**
    Shellの解析名から表示名を取得する
    @param[in] parsingName Shellの解析名
    @param[out] displayName 取得した表示名
    @return true:成功 false:失敗
  */
  bool GetShellDisplayName(const std::wstring& parsingName, std::wstring& displayName)
  {
    PIDLIST_ABSOLUTE pidl = nullptr;
    const HRESULT parseResult = SHParseDisplayName(parsingName.c_str(), nullptr, &pidl, 0, nullptr);
    std::unique_ptr<ITEMIDLIST, PidlDeleter> pidlOwner(pidl);
    if (FAILED(parseResult) || pidlOwner == nullptr)
    {
      return false;
    }

    PWSTR shellDisplayName = nullptr;
    const HRESULT nameResult = SHGetNameFromIDList(pidlOwner.get(), SIGDN_NORMALDISPLAY, &shellDisplayName);
    std::unique_ptr<wchar_t, CoTaskMemDeleter> displayNameOwner(shellDisplayName);
    if (FAILED(nameResult) || displayNameOwner == nullptr || displayNameOwner.get()[0] == L'\0')
    {
      return false;
    }

    displayName.assign(displayNameOwner.get());
    return true;
  }
}

// ヘッダーより短いファイルは解析失敗として扱う
TEST(AutomaticDestinationsFileTest, OpenReturnsFalseForShortFile)
{
	TCHAR tempDirectory[MAX_PATH]{};
	ASSERT_NE(0, GetTempPath(_countof(tempDirectory), tempDirectory));

	TCHAR filePath[MAX_PATH]{};
	ASSERT_NE(0, GetTempFileName(tempDirectory, _T("sdt"), 0, filePath));

	HANDLE file = CreateFile(filePath, GENERIC_WRITE, 0, nullptr, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	ASSERT_NE(INVALID_HANDLE_VALUE, file);
	const BYTE data[] = { 1, 2, 3, 4 };
	DWORD written = 0;
	BOOL writeResult = WriteFile(file, data, sizeof(data), &written, nullptr);
	CloseHandle(file);
	ASSERT_TRUE(writeResult);

	AutomaticDestinationsFile destinations;
	EXPECT_FALSE(destinations.Open(filePath));
  EXPECT_EQ(0, destinations.GetPinnedEntryCount());
  DeleteFile(filePath);
}

// .lnkのSystem.TitleがDestListのパスや説明文字列より優先される
TEST(AutomaticDestinationsFileTest, SystemTitleIsUsedAsDisplayPath)
{
  ComApartment comApartment;
  ASSERT_TRUE(comApartment.IsAvailable());

  std::vector<std::uint8_t> linkData;
  ASSERT_TRUE(CreateShellLinkData(L"タイトル", L"説明", linkData));

  TCHAR tempDirectory[MAX_PATH]{};
  ASSERT_NE(0, GetTempPath(_countof(tempDirectory), tempDirectory));
  TCHAR filePath[MAX_PATH]{};
  ASSERT_NE(0, GetTempFileName(tempDirectory, _T("sdt"), 0, filePath));
  ASSERT_TRUE(CreateAutomaticDestinationsFile(filePath, L"C:\\fallback.txt", linkData));

  AutomaticDestinationsFile destinations;
  ASSERT_TRUE(destinations.Open(filePath));
  ASSERT_EQ(1, destinations.GetPinnedEntryCount());

  launcherapp::commands::shell_destinations::AutomaticDestinationEntry entry{};
  ASSERT_TRUE(destinations.GetPinnedEntry(0, entry));
  EXPECT_EQ(L"タイトル", entry.displayPath);
  DeleteFile(filePath);
}

// System.Titleがない場合は従来どおり.lnkの説明文字列を表示名にする
TEST(AutomaticDestinationsFileTest, DescriptionIsUsedWhenSystemTitleIsMissing)
{
  ComApartment comApartment;
  ASSERT_TRUE(comApartment.IsAvailable());

  std::vector<std::uint8_t> linkData;
  ASSERT_TRUE(CreateShellLinkData(nullptr, L"説明", linkData));

  TCHAR tempDirectory[MAX_PATH]{};
  ASSERT_NE(0, GetTempPath(_countof(tempDirectory), tempDirectory));
  TCHAR filePath[MAX_PATH]{};
  ASSERT_NE(0, GetTempFileName(tempDirectory, _T("sdt"), 0, filePath));
  ASSERT_TRUE(CreateAutomaticDestinationsFile(filePath, L"C:\\fallback.txt", linkData));

  AutomaticDestinationsFile destinations;
  ASSERT_TRUE(destinations.Open(filePath));
  ASSERT_EQ(1, destinations.GetPinnedEntryCount());

  launcherapp::commands::shell_destinations::AutomaticDestinationEntry entry{};
  ASSERT_TRUE(destinations.GetPinnedEntry(0, entry));
  EXPECT_EQ(L"説明", entry.displayPath);
  DeleteFile(filePath);
}

// Titleと説明がないknownfolder形式はShellの表示名に変換する
TEST(AutomaticDestinationsFileTest, KnownFolderParsingNameIsResolved)
{
  ComApartment comApartment;
  ASSERT_TRUE(comApartment.IsAvailable());

  constexpr wchar_t knownFolderParsingName[] =
    L"knownfolder:{B4BFCC3A-DB2C-424C-B029-7FE99A87C641}";
  std::wstring expectedDisplayName;
  ASSERT_TRUE(GetShellDisplayName(knownFolderParsingName, expectedDisplayName));

  std::vector<std::uint8_t> linkData;
  ASSERT_TRUE(CreateShellLinkData(nullptr, L"", linkData));

  TCHAR tempDirectory[MAX_PATH]{};
  ASSERT_NE(0, GetTempPath(_countof(tempDirectory), tempDirectory));
  TCHAR filePath[MAX_PATH]{};
  ASSERT_NE(0, GetTempFileName(tempDirectory, _T("sdt"), 0, filePath));
  ASSERT_TRUE(CreateAutomaticDestinationsFile(filePath,
    std::wstring(knownFolderParsingName) + L" ", linkData));

  AutomaticDestinationsFile destinations;
  ASSERT_TRUE(destinations.Open(filePath));
  ASSERT_EQ(1, destinations.GetPinnedEntryCount());

  launcherapp::commands::shell_destinations::AutomaticDestinationEntry entry{};
  ASSERT_TRUE(destinations.GetPinnedEntry(0, entry));
  EXPECT_EQ(expectedDisplayName, entry.displayPath);
  EXPECT_NE(knownFolderParsingName, entry.displayPath);
  DeleteFile(filePath);
}

// knownfolder形式が解決できない場合はDestListの文字列を維持する
TEST(AutomaticDestinationsFileTest, UnresolvableKnownFolderParsingNameIsPreserved)
{
  ComApartment comApartment;
  ASSERT_TRUE(comApartment.IsAvailable());

  std::vector<std::uint8_t> linkData;
  ASSERT_TRUE(CreateShellLinkData(nullptr, L"", linkData));

  const std::wstring parsingName = L"knownfolder:not-a-guid ";
  TCHAR tempDirectory[MAX_PATH]{};
  ASSERT_NE(0, GetTempPath(_countof(tempDirectory), tempDirectory));
  TCHAR filePath[MAX_PATH]{};
  ASSERT_NE(0, GetTempFileName(tempDirectory, _T("sdt"), 0, filePath));
  ASSERT_TRUE(CreateAutomaticDestinationsFile(filePath, parsingName, linkData));

  AutomaticDestinationsFile destinations;
  ASSERT_TRUE(destinations.Open(filePath));
  ASSERT_EQ(1, destinations.GetPinnedEntryCount());

  launcherapp::commands::shell_destinations::AutomaticDestinationEntry entry{};
  ASSERT_TRUE(destinations.GetPinnedEntry(0, entry));
  EXPECT_EQ(parsingName, entry.displayPath);
  DeleteFile(filePath);
}
