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
  /** 解析エラーの内容を保持する例外を作成する
    @param[in] message エラーメッセージ
  */
  explicit OleCfParseException(const std::string& message);
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

  /**
    セクターIDからファイル内のオフセットを計算する
    @param[in] sectorId セクターID
    @return ファイル内のオフセット。不正なIDの場合は0
  */
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
  /**
    ファイルパスを指定して複合ファイルを開く
    @param[in] filePath 読み込むファイルのパス
  */
  explicit OleCfFile(const wchar_t* filePath);

  /**
    メモリ上のバイト列から複合ファイルを開く
    @param[in] rawBytes ファイルの内容
    @param[in] sourceFile エラー表示に使用するファイル名
  */
  OleCfFile(std::vector<std::uint8_t> rawBytes, std::wstring sourceFile = {});

  /** 複合ファイルの解析データを破棄する */
  ~OleCfFile();

  /**
    解析データの所有権を移動する
    @param[in] other 移動元
  */
  OleCfFile(OleCfFile&& other) noexcept;

  /**
    解析データの所有権を移動代入する
    @param[in] other 移動元
    @return *this
  */
  OleCfFile& operator=(OleCfFile&& other) noexcept;

  OleCfFile(const OleCfFile&) = delete;
  OleCfFile& operator=(const OleCfFile&) = delete;

  /**
    解析したディレクトリエントリ一覧を取得する
    @return ディレクトリエントリ一覧
  */
  const std::vector<OleCfDirectoryEntry>& Directory() const noexcept;

  /**
    指定したストリームのペイロードを取得する
    @param[in] directory ペイロードを取得するディレクトリエントリ
    @return ペイロード。取得できない場合は空
  */
  std::optional<std::vector<std::uint8_t>> GetPayloadForDirectory(const OleCfDirectoryEntry& directory);

private:
  struct PImpl;
  std::unique_ptr<PImpl> in_;
};

}}}
