#include "pch.h"
#include "OleCfFile.h"

#define NOMINMAX
#include <Windows.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <sstream>
#include <system_error>
#include <unordered_set>
#include <utility>

namespace launcherapp { namespace commands { namespace shell_destinations {

namespace
{
  constexpr std::size_t HeaderSize = 512;
  constexpr std::size_t DirectoryEntrySize = 128;
  // FILETIMEで表現できる最大日時（9999年12月31日23時59分59.9999999秒）の値
  constexpr std::uint64_t MaximumFileTime = 2650467743999999999ULL;
  // FILETIMEの基準日時（1601年1月1日）から365日分の100ナノ秒単位。これ未満の日時は未設定として扱う
  constexpr std::uint64_t TicksInYear1601 = 3153600000000000ULL;

  /**
    指定範囲がファイルデータ内に収まることを確認する
    @param[in] bytes 検査対象のデータ
    @param[in] offset 範囲の開始位置
    @param[in] count 範囲の長さ
    @param[in] field 解析中のフィールド名
  */
  void EnsureRange(const std::vector<std::uint8_t>& bytes, std::uint64_t offset, std::uint64_t count,
    const char* field)
  {
    if (offset > bytes.size() || count > bytes.size() - offset)
    {
      throw OleCfParseException(std::string("Read outside the file while parsing ") + field);
    }
  }

  std::uint16_t ReadUInt16(const std::vector<std::uint8_t>& bytes, std::size_t offset)
  {
    EnsureRange(bytes, offset, 2, "Header");
    return static_cast<std::uint16_t>(bytes[offset]) |
      static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[offset + 1]) << 8);
  }

  std::int16_t ReadInt16(const std::vector<std::uint8_t>& bytes, std::size_t offset)
  {
    return std::bit_cast<std::int16_t>(ReadUInt16(bytes, offset));
  }

  std::uint32_t ReadUInt32(const std::vector<std::uint8_t>& bytes, std::size_t offset)
  {
    EnsureRange(bytes, offset, 4, "Header");
    return static_cast<std::uint32_t>(bytes[offset]) |
      (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
      (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
      (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
  }

  std::int32_t ReadInt32(const std::vector<std::uint8_t>& bytes, std::size_t offset)
  {
    return std::bit_cast<std::int32_t>(ReadUInt32(bytes, offset));
  }

  std::string WideToUtf8(const std::wstring& value)
  {
    if (value.empty())
    {
      return {};
    }

    const int sourceLength = static_cast<int>(value.size());
    const int required = WideCharToMultiByte(CP_UTF8, 0, value.data(), sourceLength, nullptr, 0, nullptr, nullptr);
    if (required <= 0)
    {
      return {};
    }

    std::string result(static_cast<std::size_t>(required), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.data(), sourceLength, result.data(), required, nullptr, nullptr);
    return result;
  }

  std::string DecodeUtf16Le(const std::uint8_t* bytes, std::size_t byteCount)
  {
    std::wstring wide;
    wide.reserve(byteCount / 2);
    for (std::size_t i = 0; i + 1 < byteCount; i += 2)
    {
      const auto codeUnit = static_cast<std::uint16_t>(bytes[i]) |
        static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[i + 1]) << 8);
      wide.push_back(static_cast<wchar_t>(codeUnit));
    }
    return WideToUtf8(wide);
  }

  std::wstring CopyPath(const wchar_t* filePath)
  {
    if (filePath == nullptr)
    {
      throw std::invalid_argument("The file path must not be null");
    }
    return filePath;
  }

  struct FileHandle
  {
    explicit FileHandle(HANDLE value) : Value(value)
    {
    }

    ~FileHandle()
    {
      if (Value != INVALID_HANDLE_VALUE)
      {
        CloseHandle(Value);
      }
    }

    HANDLE Value;
  };

  /**
    ファイル全体をバイト列として読み込む
    @param[in] filePath 読み込むファイルのパス
    @return ファイルの内容
  */
  std::vector<std::uint8_t> ReadFileBytes(const wchar_t* filePath)
  {
    if (filePath == nullptr)
    {
      throw std::invalid_argument("The file path must not be null");
    }

    FileHandle file(CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL, nullptr));
    if (file.Value == INVALID_HANDLE_VALUE)
    {
      throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), WideToUtf8(filePath));
    }

    LARGE_INTEGER fileSize{};
    if (!GetFileSizeEx(file.Value, &fileSize))
    {
      throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), WideToUtf8(filePath));
    }
    if (fileSize.QuadPart < 0 || static_cast<std::uint64_t>(fileSize.QuadPart) >
      static_cast<std::uint64_t>(std::vector<std::uint8_t>().max_size()))
    {
      throw std::length_error("The file is too large to read into memory");
    }

    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(fileSize.QuadPart));
    std::size_t offset = 0;
    while (offset < bytes.size())
    {
      const auto count = static_cast<DWORD>(std::min<std::size_t>(bytes.size() - offset, 1024 * 1024));
      DWORD bytesRead = 0;
      if (!ReadFile(file.Value, bytes.data() + offset, count, &bytesRead, nullptr))
      {
        throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), WideToUtf8(filePath));
      }
      if (bytesRead == 0)
      {
        throw std::system_error(ERROR_HANDLE_EOF, std::system_category(), WideToUtf8(filePath));
      }
      offset += bytesRead;
    }

    return bytes;
  }

  std::string MakeSectorError(std::int32_t sector, const std::string& name, const char* tableName,
    std::size_t tableLength)
  {
    std::ostringstream message;
    message << "Sector " << sector << " of '" << name << "' is outside the " << tableName << " ("
      << tableLength << " entries) - is this file corrupt?";
    return message.str();
  }
}

OleCfParseException::OleCfParseException(const std::string& message)
  : std::runtime_error(message)
{
}

std::int64_t OleCfHeader::SectorOffset(std::int32_t sectorId) const noexcept
{
  if (sectorId < 0 || SectorSizeAsBytes <= 0)
  {
    return 0;
  }
  return (static_cast<std::int64_t>(sectorId) + 1) * SectorSizeAsBytes;
}

struct OleCfFile::PImpl
{
  explicit PImpl(std::vector<std::uint8_t> bytes, std::wstring source)
    : mRawBytes(std::move(bytes)), mSourceFile(std::move(source))
  {
    Parse();
  }

  std::vector<std::uint8_t> mRawBytes;
  std::vector<std::uint8_t> mMiniStream;
  OleCfHeader mHeader;
  std::vector<OleCfDirectoryEntry> mDirectory;
  std::vector<std::int32_t> mSat;
  std::optional<std::vector<std::int32_t>> mSSat;
  std::wstring mSourceFile;
  std::vector<std::string> mParseErrors;

  /** 複合ファイルの各テーブルとディレクトリを順に解析する */
  void Parse()
  {
    if (mRawBytes.size() < HeaderSize)
    {
      std::ostringstream message;
      message << "The file length (" << mRawBytes.size()
        << ") is less than the length of an OLESS header (512 bytes) - this file ("
        << WideToUtf8(mSourceFile) << ") is either not an OLE structured storage document or is corrupt.";
      throw OleCfParseException(message.str());
    }

    // ヘッダーからセクター構成を読み、後続のテーブル解析に必要な情報を確定する
    std::vector<std::uint8_t> headerBytes(mRawBytes.begin(), mRawBytes.begin() + HeaderSize);
    ReadHeader(headerBytes);

    if (mHeader.TotalSATSectors > 109)
    {
      ReadMsatSectors();
    }

    // MSATから得たセクター位置を使ってSATを構築する
    ReadSat();

    if (mHeader.SSATFirstSectorId >= 0)
    {
      bool ssatTruncated = false;
      const auto ssatBytes = GetDataFromSat(mHeader.SSATFirstSectorId, "SSAT", ssatTruncated);
      if (ssatTruncated)
      {
        mParseErrors.emplace_back("The SSAT is truncated (the file ends before its last sector)");
      }

      mSSat.emplace(ssatBytes.size() / 4);
      for (std::size_t i = 0; i < mSSat->size(); ++i)
      {
        (*mSSat)[i] = ReadInt32(ssatBytes, i * 4);
      }
    }

    // ディレクトリストリームを解析し、ストリーム名と配置情報を取得する
    ReadDirectory();

    auto root = std::find_if(mDirectory.begin(), mDirectory.end(), [](const auto& entry)
    {
      return entry.DirectoryType == OleCfDirectoryEntry::DirectoryEntryTypes::RootStorage;
    });
    if (root == mDirectory.end())
    {
      root = std::find_if(mDirectory.begin(), mDirectory.end(), [](const auto& entry)
      {
        std::string name = entry.DirectoryName;
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char value)
        {
          return static_cast<char>(std::tolower(value));
        });
        return name == "root entry";
      });
    }

    // Root Storageの領域を読み込み、短いストリームの参照先を準備する
    if (root != mDirectory.end() && root->DirectorySize > 0)
    {
      bool miniTruncated = false;
      mMiniStream = GetDataFromSat(std::bit_cast<std::int32_t>(root->FirstDirectorySectorId), "Mini stream",
        miniTruncated);
      if (miniTruncated && mMiniStream.size() < static_cast<std::size_t>(root->DirectorySize))
      {
        std::ostringstream message;
        message << "The mini stream is truncated: " << mMiniStream.size() << " of " << root->DirectorySize
          << " bytes are in the file";
        mParseErrors.push_back(message.str());
      }
    }
  }

  /** ヘッダーの形式とセクター配置情報を検証して読み込む */
  void ReadHeader(const std::vector<std::uint8_t>& bytes)
  {
    constexpr std::array<std::uint8_t, 8> signature = { 0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1 };
    if (!std::equal(signature.begin(), signature.end(), bytes.begin()))
    {
      throw OleCfParseException("Invalid signature!");
    }

    std::copy_n(bytes.begin() + 8, mHeader.ClassIdBytes.size(), mHeader.ClassIdBytes.begin());
    mHeader.RevisionMinor = ReadInt16(bytes, 24);
    mHeader.RevisionMajor = ReadInt16(bytes, 26);
    mHeader.IsLittleEndian = ReadUInt16(bytes, 28) == 0xfffe;
    if (!mHeader.IsLittleEndian)
    {
      throw OleCfParseException("Big endian compound files are not supported");
    }

    mHeader.SectorSize = ReadInt16(bytes, 30);
    mHeader.ShortSectorSize = ReadInt16(bytes, 32);
    if (mHeader.SectorSize < 7 || mHeader.SectorSize > 16)
    {
      throw OleCfParseException("Invalid sector size shift " + std::to_string(mHeader.SectorSize));
    }
    if (mHeader.ShortSectorSize < 1 || mHeader.ShortSectorSize >= mHeader.SectorSize)
    {
      throw OleCfParseException("Invalid short sector size shift " + std::to_string(mHeader.ShortSectorSize));
    }

    mHeader.SectorSizeAsBytes = 1 << mHeader.SectorSize;
    mHeader.ShortSectorSizeAsBytes = 1 << mHeader.ShortSectorSize;
    mHeader.TotalSATSectors = ReadInt32(bytes, 44);
    mHeader.DirectoryStreamFirstSectorId = ReadInt32(bytes, 48);
    mHeader.MinimumStandardStreamSize = ReadUInt32(bytes, 56);
    mHeader.SSATFirstSectorId = ReadInt32(bytes, 60);
    mHeader.TotalSSATSectors = ReadUInt32(bytes, 64);
    mHeader.MSATFirstSectorId = ReadInt32(bytes, 68);
    mHeader.MSATTotalSectors = ReadInt32(bytes, 72);

    const auto maximumSatSectors = mRawBytes.size() / static_cast<std::size_t>(mHeader.SectorSizeAsBytes) + 1;
    if (mHeader.TotalSATSectors < 0 || static_cast<std::uint64_t>(mHeader.TotalSATSectors) > maximumSatSectors)
    {
      throw OleCfParseException("Invalid number of SAT sectors (" + std::to_string(mHeader.TotalSATSectors) +
        ") - is this file corrupt?");
    }

    // ヘッダー内に格納された先頭109個のSATセクターIDを読み取る
    mHeader.SATSectors.resize(static_cast<std::size_t>(mHeader.TotalSATSectors));
    mHeader.SectorIds.reserve(109);
    for (std::int32_t i = 0; i < 109; ++i)
    {
      const auto satAddress = ReadInt32(bytes, 76 + static_cast<std::size_t>(i) * 4);
      if (satAddress >= 0)
      {
        if (static_cast<std::size_t>(i) >= mHeader.SATSectors.size())
        {
          std::ostringstream message;
          message << "The current sector (" << i
            << ") was larger than the total number of sectors expected from the header data ("
            << mHeader.TotalSATSectors << ") - is this file corrupt?";
          throw OleCfParseException(message.str());
        }
        mHeader.SATSectors[static_cast<std::size_t>(i)] = mHeader.SectorOffset(satAddress);
      }
      mHeader.SectorIds.push_back(satAddress);
    }
  }

  /** ヘッダーに収まらない追加のSATセクターIDをMSATから読み込む */
  void ReadMsatSectors()
  {
    const auto sectorSize = static_cast<std::size_t>(mHeader.SectorSizeAsBytes);
    const auto idsPerSector = sectorSize / 4 - 1;
    auto remaining = mHeader.TotalSATSectors - 109;
    auto satIndex = std::size_t{ 109 };
    auto msatSector = mHeader.MSATFirstSectorId;
    std::unordered_set<std::int32_t> visited;

    while (remaining > 0)
    {
      const auto offset = mHeader.SectorOffset(msatSector);
      if (msatSector < 0 || !visited.insert(msatSector).second || offset < 0 ||
        static_cast<std::uint64_t>(offset) + sectorSize > mRawBytes.size())
      {
        std::ostringstream message;
        message << "MSAT sector " << msatSector << " is invalid, repeated or past the end of the file, with "
          << remaining << " SAT sector ids still to read - is this file corrupt?";
        throw OleCfParseException(message.str());
      }

      const auto count = std::min<std::int32_t>(remaining, static_cast<std::int32_t>(idsPerSector));
      for (std::int32_t i = 0; i < count; ++i)
      {
        const auto satAddress = ReadInt32(mRawBytes, static_cast<std::size_t>(offset) +
          static_cast<std::size_t>(i) * 4);
        if (satAddress >= 0)
        {
          mHeader.SATSectors[satIndex] = mHeader.SectorOffset(satAddress);
        }
        mHeader.SectorIds.push_back(satAddress);
        ++satIndex;
      }

      remaining -= count;
      msatSector = ReadInt32(mRawBytes, static_cast<std::size_t>(offset) + idsPerSector * 4);
    }
  }

  /** SATセクターを読み、セクター連鎖をたどるための表を作成する */
  void ReadSat()
  {
    const auto sectorSize = static_cast<std::size_t>(mHeader.SectorSizeAsBytes);
    const auto entriesPerSector = sectorSize / 4;
    mSat.resize(static_cast<std::size_t>(mHeader.TotalSATSectors) * entriesPerSector);

    for (std::size_t i = 0; i < mHeader.SATSectors.size(); ++i)
    {
      const auto offset = mHeader.SATSectors[i];
      if (offset == 0 || offset < 0 || static_cast<std::uint64_t>(offset) + sectorSize > mRawBytes.size())
      {
        if (offset == 0)
        {
          mParseErrors.push_back("SAT sector " + std::to_string(i) + " is missing");
        }
        else
        {
          std::ostringstream message;
          message << "SAT sector " << i << " at 0x" << std::hex << offset << " is past the end of the file";
          mParseErrors.push_back(message.str());
        }

        std::fill_n(mSat.begin() + static_cast<std::ptrdiff_t>(i * entriesPerSector), entriesPerSector, -1);
        continue;
      }

      const auto sourceOffset = static_cast<std::size_t>(offset);
      for (std::size_t j = 0; j < entriesPerSector; ++j)
      {
        mSat[i * entriesPerSector + j] = ReadInt32(mRawBytes, sourceOffset + j * 4);
      }
    }
  }

  /** 128バイトのディレクトリレコードを構造体に変換する */
  OleCfDirectoryEntry ReadDirectoryEntry(const std::uint8_t* bytes, std::int32_t id)
  {
    OleCfDirectoryEntry entry;
    entry.Id = id;
    const auto rawNameLength = static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[64]) |
      static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[65]) << 8));
    auto nameLength = std::bit_cast<std::int16_t>(rawNameLength);
    if (nameLength < 2 || nameLength > 64 || nameLength % 2 != 0)
    {
      entry.ParseErrors.push_back("Invalid name length " + std::to_string(nameLength));
      nameLength = std::max<std::int16_t>(2, std::min<std::int16_t>(64,
        static_cast<std::int16_t>(nameLength & ~1)));
    }

    entry.DirectoryName = DecodeUtf16Le(bytes, static_cast<std::size_t>(nameLength - 2));
    entry.DirectoryType = static_cast<OleCfDirectoryEntry::DirectoryEntryTypes>(bytes[66]);
    entry.NodeColor = static_cast<OleCfDirectoryEntry::NodeColors>(bytes[67]);
    entry.PreviousDirectoryId = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(bytes[68]) |
      (static_cast<std::uint32_t>(bytes[69]) << 8) | (static_cast<std::uint32_t>(bytes[70]) << 16) |
      (static_cast<std::uint32_t>(bytes[71]) << 24));
    entry.NextDirectoryId = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(bytes[72]) |
      (static_cast<std::uint32_t>(bytes[73]) << 8) | (static_cast<std::uint32_t>(bytes[74]) << 16) |
      (static_cast<std::uint32_t>(bytes[75]) << 24));
    entry.SubDirectoryId = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(bytes[76]) |
      (static_cast<std::uint32_t>(bytes[77]) << 8) | (static_cast<std::uint32_t>(bytes[78]) << 16) |
      (static_cast<std::uint32_t>(bytes[79]) << 24));
    std::copy_n(bytes + 80, entry.ClassIdBytes.size(), entry.ClassIdBytes.begin());
    entry.UserFlags = static_cast<std::uint32_t>(bytes[96]) | (static_cast<std::uint32_t>(bytes[97]) << 8) |
      (static_cast<std::uint32_t>(bytes[98]) << 16) | (static_cast<std::uint32_t>(bytes[99]) << 24);
    entry.CreationTimeFileTime = ReadDirectoryTime(bytes, 100, "Creation time", entry.ParseErrors);
    entry.ModifiedTimeFileTime = ReadDirectoryTime(bytes, 108, "Modified time", entry.ParseErrors);
    entry.FirstDirectorySectorId = static_cast<std::uint32_t>(bytes[116]) |
      (static_cast<std::uint32_t>(bytes[117]) << 8) | (static_cast<std::uint32_t>(bytes[118]) << 16) |
      (static_cast<std::uint32_t>(bytes[119]) << 24);
    entry.DirectorySize = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(bytes[120]) |
      (static_cast<std::uint32_t>(bytes[121]) << 8) | (static_cast<std::uint32_t>(bytes[122]) << 16) |
      (static_cast<std::uint32_t>(bytes[123]) << 24));
    return entry;
  }

  /** ディレクトリレコードのFILETIMEを検証して読み取る */
  std::optional<std::int64_t> ReadDirectoryTime(const std::uint8_t* bytes, std::size_t offset,
    const char* field, std::vector<std::string>& errors)
  {
    std::uint64_t rawValue = 0;
    for (std::size_t i = 0; i < 8; ++i)
    {
      rawValue |= static_cast<std::uint64_t>(bytes[offset + i]) << (i * 8);
    }
    const auto value = std::bit_cast<std::int64_t>(rawValue);
    if (value == 0)
    {
      return std::nullopt;
    }
    if (value < 0 || static_cast<std::uint64_t>(value) > MaximumFileTime)
    {
      std::ostringstream message;
      message << field << " is out of range (0x" << std::hex << std::uppercase << rawValue << ")";
      errors.push_back(message.str());
      return std::nullopt;
    }
    if (static_cast<std::uint64_t>(value) < TicksInYear1601)
    {
      return std::nullopt;
    }
    return value;
  }

  /** ディレクトリストリームをレコード単位で解析する */
  void ReadDirectory()
  {
    bool directoryTruncated = false;
    const auto directoryBytes = GetDataFromSat(mHeader.DirectoryStreamFirstSectorId, "Directory", directoryTruncated);
    if (directoryTruncated)
    {
      mParseErrors.emplace_back("The directory is truncated (the file ends before its last sector)");
    }

    for (std::size_t offset = 0; offset + DirectoryEntrySize <= directoryBytes.size();
      offset += DirectoryEntrySize)
    {
      const auto* bytes = directoryBytes.data() + offset;
      const auto rawNameLength = static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[64]) |
        static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[65]) << 8));
      const auto nameLength = std::bit_cast<std::int16_t>(rawNameLength);
      if (bytes[66] != 0 && nameLength > 0)
      {
        auto entry = ReadDirectoryEntry(bytes, static_cast<std::int32_t>(offset / DirectoryEntrySize));
        for (const auto& error : entry.ParseErrors)
        {
          mParseErrors.push_back("Directory entry " + std::to_string(entry.Id) + " (" + entry.DirectoryName +
            "): " + error);
        }
        mDirectory.push_back(std::move(entry));
      }
    }
  }

  /** SATのセクター連鎖から通常ストリームのデータを取り出す */
  std::vector<std::uint8_t> GetDataFromSat(std::int32_t sectorNumber, const std::string& name, bool& truncated)
  {
    const auto sectorSize = static_cast<std::size_t>(mHeader.SectorSizeAsBytes);
    const auto chain = FollowChain(mSat, sectorNumber, name, "SAT");

    std::size_t present = 0;
    while (present < chain.size() &&
      (static_cast<std::uint64_t>(chain[present]) + 1) * sectorSize < mRawBytes.size())
    {
      ++present;
    }

    std::vector<std::uint8_t> result(sectorSize * present);
    std::size_t outputOffset = 0;
    truncated = present < chain.size();
    for (std::size_t i = 0; i < present; ++i)
    {
      const auto sourceOffset = mHeader.SectorOffset(chain[i]);
      const auto readSize = static_cast<std::size_t>(std::min<std::uint64_t>(sectorSize,
        mRawBytes.size() - static_cast<std::size_t>(sourceOffset)));
      std::copy_n(mRawBytes.begin() + static_cast<std::ptrdiff_t>(sourceOffset), readSize,
        result.begin() + static_cast<std::ptrdiff_t>(outputOffset));
      outputOffset += readSize;
      if (readSize < sectorSize)
      {
        truncated = i < chain.size() - 1 || truncated;
        break;
      }
    }

    result.resize(outputOffset);
    return result;
  }

  /** SSATのセクター連鎖からミニストリームのデータを取り出す */
  std::vector<std::uint8_t> GetDataFromSSat(std::int32_t sectorNumber, const std::string& name)
  {
    const auto sectorSize = static_cast<std::size_t>(mHeader.ShortSectorSizeAsBytes);
    const auto chain = FollowChain(*mSSat, sectorNumber, name, "SSAT");
    std::vector<std::uint8_t> result(sectorSize * chain.size());

    std::size_t outputOffset = 0;
    for (const auto sector : chain)
    {
      const auto start = static_cast<std::uint64_t>(sector) * sectorSize;
      if (start >= mMiniStream.size())
      {
        break;
      }

      const auto readSize = static_cast<std::size_t>(std::min<std::uint64_t>(sectorSize,
        mMiniStream.size() - static_cast<std::size_t>(start)));
      std::copy_n(mMiniStream.begin() + static_cast<std::ptrdiff_t>(start), readSize,
        result.begin() + static_cast<std::ptrdiff_t>(outputOffset));
      outputOffset += readSize;
      if (readSize < sectorSize)
      {
        break;
      }
    }

    result.resize(outputOffset);
    return result;
  }

  /**
    割り当て表をたどり、循環や範囲外参照を検出しながらセクター列を得る
    @param[in] table セクター連鎖を定義する割り当て表
    @param[in] first 先頭セクターID
    @param[in] name 対象ストリーム名
    @param[in] tableName 使用する割り当て表の名前
    @return 連鎖を構成するセクターID一覧
  */
  static std::vector<std::int32_t> FollowChain(const std::vector<std::int32_t>& table, std::int32_t first,
    const std::string& name, const char* tableName)
  {
    std::vector<std::int32_t> chain;
    std::unordered_set<std::int32_t> visited;
    auto sector = first;
    while (sector >= 0)
    {
      if (static_cast<std::size_t>(sector) >= table.size())
      {
        throw OleCfParseException(MakeSectorError(sector, name, tableName, table.size()));
      }
      if (!visited.insert(sector).second)
      {
        std::ostringstream message;
        message << "The " << tableName << " chain of '" << name << "' loops back to sector " << sector
          << " - is this file corrupt?";
        throw OleCfParseException(message.str());
      }
      chain.push_back(sector);
      sector = table[static_cast<std::size_t>(sector)];
    }

    if (chain.empty())
    {
      throw OleCfParseException("'" + name + "' starts at invalid sector " + std::to_string(first));
    }
    return chain;
  }

  /**
    ディレクトリエントリから通常／ミニストリームを読み分けてペイロードを取得する
    @param[in] directory ペイロードを取得するディレクトリエントリ
    @return ペイロード。データを取得できない場合は空
  */
  std::optional<std::vector<std::uint8_t>> GetPayloadForDirectory(const OleCfDirectoryEntry& directory)
  {
    if (directory.DirectorySize < 0)
    {
      throw OleCfParseException("Invalid size (" + std::to_string(directory.DirectorySize) + ") for directory (" +
        directory.DirectoryName + ")");
    }

    std::vector<std::uint8_t> bytes;
    // MinimumStandardStreamSizeを境に、SATまたはSSATの読み出し経路を選ぶ
    if (static_cast<std::uint32_t>(directory.DirectorySize) >= mHeader.MinimumStandardStreamSize)
    {
      bool ignoredTruncation = false;
      bytes = GetDataFromSat(std::bit_cast<std::int32_t>(directory.FirstDirectorySectorId),
        directory.DirectoryName, ignoredTruncation);
    }
    else if (mSSat.has_value())
    {
      if (directory.DirectorySize == 0)
      {
        return std::vector<std::uint8_t>{};
      }
      bytes = GetDataFromSSat(std::bit_cast<std::int32_t>(directory.FirstDirectorySectorId),
        directory.DirectoryName);
    }
    else
    {
      return std::nullopt;
    }

    auto length = static_cast<std::size_t>(directory.DirectorySize);
    if (length > bytes.size())
    {
      std::ostringstream message;
      message << "Stream '" << directory.DirectoryName << "' is truncated: " << bytes.size() << " of "
        << directory.DirectorySize << " bytes are in the file";
      if (std::find(mParseErrors.begin(), mParseErrors.end(), message.str()) == mParseErrors.end())
      {
        mParseErrors.push_back(message.str());
      }
      length = bytes.size();
    }

    bytes.resize(length);
    return bytes;
  }
};

OleCfFile::OleCfFile(const wchar_t* filePath)
  : in_(std::make_unique<PImpl>(ReadFileBytes(filePath), CopyPath(filePath)))
{
}

OleCfFile::OleCfFile(std::vector<std::uint8_t> rawBytes, std::wstring sourceFile)
  : in_(std::make_unique<PImpl>(std::move(rawBytes), std::move(sourceFile)))
{
}

OleCfFile::~OleCfFile() = default;

OleCfFile::OleCfFile(OleCfFile&& other) noexcept = default;

OleCfFile& OleCfFile::operator=(OleCfFile&& other) noexcept = default;

const std::vector<OleCfDirectoryEntry>& OleCfFile::Directory() const noexcept
{
  return in_->mDirectory;
}

std::optional<std::vector<std::uint8_t>> OleCfFile::GetPayloadForDirectory(
  const OleCfDirectoryEntry& directory)
{
  return in_->GetPayloadForDirectory(directory);
}

}}}
