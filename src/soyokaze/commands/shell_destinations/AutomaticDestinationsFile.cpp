#include "pch.h"
#include "AutomaticDestinationsFile.h"
#include "commands/shell_destinations/OleCfFile.h"

#define NOMINMAX
#include <objbase.h>
#include <propsys.h>
#include <propvarutil.h>
#include <shlobj.h>
#include <shobjidl.h>

#include <wrl/client.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <cstdint>
#include <cwctype>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace launcherapp { namespace commands { namespace shell_destinations {

namespace
{
  constexpr std::size_t DestListHeaderSize = 32;
  constexpr std::size_t ShellLinkHeaderSize = 0x4c;
  constexpr std::size_t ShellLinkFileAttributesOffset = 0x18;
  constexpr std::size_t Version1PathOffset = 112;
  constexpr std::size_t LaterVersionPathOffset = 128;

  void EnsureRange(const std::vector<std::uint8_t>& bytes, std::size_t offset, std::size_t length)
  {
    if (offset > bytes.size() || length > bytes.size() - offset)
    {
      throw std::runtime_error("DestList のデータが途中で終了しています。");
    }
  }

  std::uint16_t ReadUInt16(const std::vector<std::uint8_t>& bytes, std::size_t offset)
  {
    EnsureRange(bytes, offset, 2);
    return static_cast<std::uint16_t>(bytes[offset]) |
      static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[offset + 1]) << 8);
  }

  std::uint32_t ReadUInt32(const std::vector<std::uint8_t>& bytes, std::size_t offset)
  {
    EnsureRange(bytes, offset, 4);
    return static_cast<std::uint32_t>(bytes[offset]) |
      (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
      (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
      (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
  }

  std::int32_t ReadInt32(const std::vector<std::uint8_t>& bytes, std::size_t offset)
  {
    return std::bit_cast<std::int32_t>(ReadUInt32(bytes, offset));
  }

  std::wstring ReadUtf16String(
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    std::size_t characterCount)
  {
    if (characterCount > (std::numeric_limits<std::size_t>::max)() / 2)
    {
      throw std::runtime_error("DestList の文字列長が不正です。");
    }
    EnsureRange(bytes, offset, characterCount * 2);

    std::wstring value;
    value.reserve(characterCount);
    for (std::size_t i = 0; i < characterCount; ++i)
    {
      value.push_back(static_cast<wchar_t>(ReadUInt16(bytes, offset + i * 2)));
    }
    return value;
  }

  struct DestListEntry
  {
    std::int32_t EntryNumber = -1;
    std::int32_t PinStatus = -1;
    std::wstring Path;
  };

  struct ParsedDestList
  {
    std::int32_t Version = 0;
    std::int32_t NumberOfEntries = 0;
    std::int32_t NumberOfPinnedEntries = 0;
    std::vector<DestListEntry> Entries;
  };

  ParsedDestList ParseDestList(const std::vector<std::uint8_t>& bytes)
  {
    if (bytes.size() < DestListHeaderSize)
    {
      throw std::runtime_error("DestList ヘッダーが32バイト未満です。");
    }

    ParsedDestList result;
    result.Version = ReadInt32(bytes, 0);
    result.NumberOfEntries = ReadInt32(bytes, 4);
    result.NumberOfPinnedEntries = ReadInt32(bytes, 8);
    if (result.Version < 1 || result.NumberOfEntries < 0 || result.NumberOfPinnedEntries < 0)
    {
      throw std::runtime_error("DestList ヘッダーの値が不正です。");
    }

    const std::size_t pathOffset = result.Version == 1
      ? Version1PathOffset
      : LaterVersionPathOffset;
    std::size_t offset = DestListHeaderSize;

    while (offset < bytes.size() &&
      (result.Version != 1 || result.Entries.size() < static_cast<std::size_t>(result.NumberOfEntries)))
    {
      const std::size_t remaining = bytes.size() - offset;
      if (remaining < pathOffset + 2)
      {
        throw std::runtime_error("DestList 項目の固定部分が途中で終了しています。");
      }

      const std::int16_t pathCharacterCount = std::bit_cast<std::int16_t>(ReadUInt16(bytes, offset + pathOffset));
      if (pathCharacterCount < 0)
      {
        throw std::runtime_error("DestList 項目のパス長が不正です。");
      }

      const std::size_t pathBytes = static_cast<std::size_t>(pathCharacterCount) * 2;
      const std::size_t entrySize = pathOffset + 2 + pathBytes;
      if (entrySize > remaining)
      {
        throw std::runtime_error("DestList 項目のパスが途中で終了しています。");
      }

      std::size_t stride = entrySize;
      if (result.Version != 1)
      {
        if (remaining - entrySize < 4)
        {
          throw std::runtime_error("DestList 項目にプロパティストア長がありません。");
        }

        const std::int32_t propertyStoreSize = ReadInt32(bytes, offset + entrySize);
        if (propertyStoreSize < 0 ||
          static_cast<std::size_t>(propertyStoreSize) > remaining - entrySize - 4)
        {
          throw std::runtime_error("DestList 項目のプロパティストア長が不正です。");
        }
        stride += 4 + static_cast<std::size_t>(propertyStoreSize);
      }

      DestListEntry entry;
      entry.EntryNumber = ReadInt32(bytes, offset + 88);
      entry.PinStatus = ReadInt32(bytes, offset + 108);
      entry.Path = ReadUtf16String(bytes, offset + pathOffset + 2,
        static_cast<std::size_t>(pathCharacterCount));
      if (entry.EntryNumber < 0)
      {
        throw std::runtime_error("DestList 項目IDが負の値です。");
      }

      result.Entries.push_back(std::move(entry));
      offset += stride;
    }

    if (result.Version == 1 &&
      result.Entries.size() != static_cast<std::size_t>(result.NumberOfEntries))
    {
      throw std::runtime_error("DestList ヘッダーの項目数と実際の項目数が一致しません。");
    }

    return result;
  }

  bool EqualsAsciiIgnoreCase(const std::string& left, const std::string& right)
  {
    if (left.size() != right.size())
    {
      return false;
    }

    for (std::size_t i = 0; i < left.size(); ++i)
    {
      const auto leftChar = static_cast<unsigned char>(left[i]);
      const auto rightChar = static_cast<unsigned char>(right[i]);
      if (std::tolower(leftChar) != std::tolower(rightChar))
      {
        return false;
      }
    }
    return true;
  }

  std::string EntryNumberToStreamName(std::int32_t entryNumber)
  {
    std::ostringstream streamName;
    streamName << std::uppercase << std::hex << entryNumber;
    return streamName.str();
  }

  const OleCfDirectoryEntry* FindStream(const OleCfFile& file, const std::string& streamName)
  {
    for (const auto& directory : file.Directory())
    {
      if (directory.DirectoryType == OleCfDirectoryEntry::DirectoryEntryTypes::Stream &&
        EqualsAsciiIgnoreCase(directory.DirectoryName, streamName))
      {
        return &directory;
      }
    }
    return nullptr;
  }

  const OleCfDirectoryEntry* FindDestListStream(const OleCfFile& file)
  {
    for (const auto& directory : file.Directory())
    {
      if (directory.DirectoryType == OleCfDirectoryEntry::DirectoryEntryTypes::Stream &&
        EqualsAsciiIgnoreCase(directory.DirectoryName, "DestList"))
      {
        return &directory;
      }
    }
    return nullptr;
  }

  class ComApartment
  {
  public:
    ComApartment()
      : result_(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))
    {
    }

    ~ComApartment()
    {
      if (SUCCEEDED(result_))
      {
        CoUninitialize();
      }
    }

    bool IsAvailable() const noexcept
    {
      return SUCCEEDED(result_) || result_ == RPC_E_CHANGED_MODE;
    }

  private:
    HRESULT result_;
  };

  bool IsValidShellLink(const std::vector<std::uint8_t>& bytes)
  {
    constexpr std::array<std::uint8_t, 16> shellLinkClassId =
    {
      0x01, 0x14, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
      0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46
    };

    if (bytes.size() < ShellLinkHeaderSize || ReadUInt32(bytes, 0) != ShellLinkHeaderSize)
    {
      return false;
    }

    return std::equal(shellLinkClassId.begin(), shellLinkClassId.end(), bytes.begin() + 4);
  }

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

  /**
    knownfolder形式の解析名をShell上の表示名に変換する
    @param[in] value 変換対象の解析名
    @param[out] displayName 取得した表示名
    @return true:変換成功 false:対象外または変換失敗
  */
  bool TryGetKnownFolderDisplayName(const std::wstring& value, std::wstring& displayName)
  {
    constexpr wchar_t prefix[] = L"knownfolder:";
    constexpr std::size_t prefixLength = std::size(prefix) - 1;

    std::wstring parsingName(value);
    while (!parsingName.empty() && std::iswspace(parsingName.back()) != 0) {
      parsingName.pop_back();
    }
    if (parsingName.size() <= prefixLength ||
      CompareStringOrdinal(parsingName.data(), static_cast<int>(prefixLength),
        prefix, static_cast<int>(prefixLength), TRUE) != CSTR_EQUAL) {
      return false;
    }

    CLSID knownFolderId{};
    if (FAILED(CLSIDFromString(parsingName.c_str() + prefixLength, &knownFolderId))) {
      return false;
    }

    PIDLIST_ABSOLUTE pidl = nullptr;
    const HRESULT parseResult = SHParseDisplayName(parsingName.c_str(), nullptr, &pidl, 0, nullptr);
    std::unique_ptr<ITEMIDLIST, PidlDeleter> pidlOwner(pidl);
    if (FAILED(parseResult) || pidlOwner == nullptr) {
      return false;
    }

    PWSTR shellDisplayName = nullptr;
    const HRESULT nameResult = SHGetNameFromIDList(pidlOwner.get(), SIGDN_NORMALDISPLAY, &shellDisplayName);
    std::unique_ptr<wchar_t, CoTaskMemDeleter> displayNameOwner(shellDisplayName);
    if (FAILED(nameResult) || displayNameOwner == nullptr || displayNameOwner.get()[0] == L'\0') {
      return false;
    }

    displayName.assign(displayNameOwner.get());
    return true;
  }

  std::wstring ReadShellLinkNameString(const std::vector<std::uint8_t>& bytes)
  {
    if (bytes.size() > (std::numeric_limits<ULONG>::max)())
    {
      return {};
    }

    Microsoft::WRL::ComPtr<IShellLinkW> shellLink;
    if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
      IID_PPV_ARGS(shellLink.GetAddressOf()))))
    {
      return {};
    }

    Microsoft::WRL::ComPtr<IPersistStream> persistStream;
    if (FAILED(shellLink.As(&persistStream)))
    {
      return {};
    }

    Microsoft::WRL::ComPtr<IStream> stream;
    if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, stream.GetAddressOf())))
    {
      return {};
    }

    ULONG bytesWritten = 0;
    if (FAILED(stream->Write(bytes.data(), static_cast<ULONG>(bytes.size()), &bytesWritten)) ||
      bytesWritten != bytes.size())
    {
      return {};
    }

    LARGE_INTEGER streamStart{};
    if (FAILED(stream->Seek(streamStart, STREAM_SEEK_SET, nullptr)) ||
      FAILED(persistStream->Load(stream.Get())))
    {
      return {};
    }

    std::vector<wchar_t> buffer(32768, L'\0');

    // System.Titleを優先し、取得できない場合は従来の説明文字列を使う
    PROPERTYKEY titleKey{};
    if (SUCCEEDED(PSGetPropertyKeyFromName(L"System.Title", &titleKey)))
    {
      Microsoft::WRL::ComPtr<IPropertyStore> propertyStore;
      if (SUCCEEDED(shellLink.As(&propertyStore)))
      {
        PROPVARIANT titleValue{};
        if (SUCCEEDED(propertyStore->GetValue(titleKey, &titleValue)))
        {
          const HRESULT convertResult = PropVariantToString(
            titleValue, buffer.data(), static_cast<UINT>(buffer.size()));
          PropVariantClear(&titleValue);
          if (SUCCEEDED(convertResult) && buffer[0] != L'\0')
          {
            return buffer.data();
          }
        }
      }
    }

    std::fill(buffer.begin(), buffer.end(), L'\0');
    if (FAILED(shellLink->GetDescription(buffer.data(), static_cast<int>(buffer.size()))))
    {
      return {};
    }
    return buffer.data();
  }

  std::wstring GetDisplayPath(const DestListEntry& entry, const std::vector<std::uint8_t>& linkBytes)
  {
    std::wstring value = ReadShellLinkNameString(linkBytes);
    if (value.empty()) {
      value = entry.Path;
    }

    std::wstring shellDisplayName;
    return TryGetKnownFolderDisplayName(value, shellDisplayName) ? shellDisplayName : value;
  }

  bool TryCreatePinnedEntry(
    OleCfFile& file,
    const DestListEntry& destListEntry,
    AutomaticDestinationEntry& entry)
  {
    const std::string streamName = EntryNumberToStreamName(destListEntry.EntryNumber);
    const OleCfDirectoryEntry* stream = FindStream(file, streamName);
    if (stream == nullptr)
    {
      return false;
    }

    const auto payload = file.GetPayloadForDirectory(*stream);
    if (!payload || !IsValidShellLink(*payload))
    {
      return false;
    }

    entry.lnk.assign(payload->begin(), payload->end());
    entry.displayPath = GetDisplayPath(destListEntry, *payload);
    entry.isDirectory = (ReadUInt32(*payload, ShellLinkFileAttributesOffset) & FILE_ATTRIBUTE_DIRECTORY) != 0;
    return true;
  }
}


struct AutomaticDestinationsFile::PImpl
{
  std::vector<AutomaticDestinationEntry> mPinnedEntries;
};

AutomaticDestinationsFile::AutomaticDestinationsFile()
  : in(std::make_unique<PImpl>())
{
}

AutomaticDestinationsFile::~AutomaticDestinationsFile() = default;

bool AutomaticDestinationsFile::Open(const wchar_t* filePath)
{
  in->mPinnedEntries.clear();
  if (filePath == nullptr)
  {
    return false;
  }

  try
  {
    OleCfFile file(filePath);
    const OleCfDirectoryEntry* destListStream = FindDestListStream(file);
    if (destListStream == nullptr)
    {
      return false;
    }
    if (destListStream->DirectorySize == 0)
    {
      return true;
    }

    const auto destListBytes = file.GetPayloadForDirectory(*destListStream);
    if (!destListBytes)
    {
      return false;
    }

    const ParsedDestList destList = ParseDestList(*destListBytes);
    ComApartment comApartment;
    std::vector<AutomaticDestinationEntry> pinnedEntries;
    for (const auto& destListEntry : destList.Entries)
    {
      if (destListEntry.PinStatus == -1)
      {
        continue;
      }

      try
      {
        AutomaticDestinationEntry entry{};
        if (comApartment.IsAvailable() && TryCreatePinnedEntry(file, destListEntry, entry))
        {
          pinnedEntries.push_back(std::move(entry));
          continue;
        }

        if (!comApartment.IsAvailable())
        {
          const std::string streamName = EntryNumberToStreamName(destListEntry.EntryNumber);
          const OleCfDirectoryEntry* stream = FindStream(file, streamName);
          if (stream == nullptr)
          {
            continue;
          }
          const auto payload = file.GetPayloadForDirectory(*stream);
          if (!payload || !IsValidShellLink(*payload))
          {
            continue;
          }

          entry.lnk.assign(payload->begin(), payload->end());
          entry.displayPath = destListEntry.Path;
          entry.isDirectory = (ReadUInt32(*payload, ShellLinkFileAttributesOffset) & FILE_ATTRIBUTE_DIRECTORY) != 0;
          pinnedEntries.push_back(std::move(entry));
        }
      }
      catch (const std::exception&)
      {
      }
    }

    in->mPinnedEntries = std::move(pinnedEntries);
    return true;
  }
  catch (const std::exception&)
  {
    in->mPinnedEntries.clear();
    return false;
  }
  catch (...)
  {
    in->mPinnedEntries.clear();
    return false;
  }
}

size_t AutomaticDestinationsFile::GetPinnedEntryCount()
{
  return in->mPinnedEntries.size();
}

bool AutomaticDestinationsFile::GetPinnedEntry(size_t index, AutomaticDestinationEntry& entry)
{
  if (index >= in->mPinnedEntries.size())
  {
    return false;
  }

  entry = in->mPinnedEntries[index];
  return true;
}

}}}
