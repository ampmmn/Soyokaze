#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace launcherapp { namespace commands { namespace shell_destinations {


class OleCfParseException : public std::runtime_error
{
public:
  OleCfParseException(const std::string& message, std::int64_t offset = -1, std::string field = {});

  std::int64_t Offset() const noexcept;
  const std::string& Field() const noexcept;

private:
  std::int64_t offset_;
  std::string field_;
};

struct OleCfHeader
{
  std::array<std::uint8_t, 16> ClassIdBytes{};
  std::int16_t RevisionMinor = 0;
  std::int16_t RevisionMajor = 0;
  bool IsLittleEndian = false;
  std::int16_t SectorSize = 0;
  std::int32_t SectorSizeAsBytes = 0;
  std::int16_t ShortSectorSize = 0;
  std::int32_t ShortSectorSizeAsBytes = 0;
  std::int32_t TotalSATSectors = 0;
  std::int32_t DirectoryStreamFirstSectorId = 0;
  std::uint32_t MinimumStandardStreamSize = 0;
  std::int32_t SSATFirstSectorId = 0;
  std::uint32_t TotalSSATSectors = 0;
  std::int32_t MSATFirstSectorId = 0;
  std::int32_t MSATTotalSectors = 0;
  std::vector<std::int64_t> SATSectors;
  std::vector<std::int32_t> SectorIds;

  std::int64_t SectorOffset(std::int32_t sectorId) const noexcept;
};

struct OleCfDirectoryEntry
{
  enum class DirectoryEntryTypes : std::uint8_t
  {
    Empty = 0,
    Storage = 1,
    Stream = 2,
    LockBytes = 3,
    Property = 4,
    RootStorage = 5
  };

  enum class NodeColors : std::uint8_t
  {
    Red = 0,
    Black = 1
  };

  std::int32_t Id = -1;
  std::array<std::uint8_t, 16> ClassIdBytes{};
  std::uint32_t UserFlags = 0;
  std::optional<std::int64_t> CreationTimeFileTime;
  std::optional<std::int64_t> ModifiedTimeFileTime;
  std::uint32_t FirstDirectorySectorId = 0;
  std::int32_t DirectorySize = 0;
  std::int32_t PreviousDirectoryId = 0;
  std::int32_t NextDirectoryId = 0;
  std::int32_t SubDirectoryId = 0;
  std::string DirectoryName;
  DirectoryEntryTypes DirectoryType = DirectoryEntryTypes::Empty;
  NodeColors NodeColor = NodeColors::Red;
  std::vector<std::string> ParseErrors;
};

class OleCfFile
{
public:
  explicit OleCfFile(const wchar_t* filePath);
  OleCfFile(std::vector<std::uint8_t> rawBytes, std::wstring sourceFile = {});
  ~OleCfFile();

  OleCfFile(OleCfFile&& other) noexcept;
  OleCfFile& operator=(OleCfFile&& other) noexcept;

  OleCfFile(const OleCfFile&) = delete;
  OleCfFile& operator=(const OleCfFile&) = delete;

  const OleCfHeader& Header() const noexcept;
  const std::vector<OleCfDirectoryEntry>& Directory() const noexcept;
  const std::vector<std::int32_t>& Sat() const noexcept;
  const std::optional<std::vector<std::int32_t>>& SSat() const noexcept;
  const std::wstring& SourceFile() const noexcept;
  const std::vector<std::string>& ParseErrors() const noexcept;
  bool ParsingError() const noexcept;

  std::optional<std::vector<std::uint8_t>> GetPayloadForDirectory(const OleCfDirectoryEntry& directory);

private:
  struct PImpl;
  std::unique_ptr<PImpl> in_;
};

}}}
