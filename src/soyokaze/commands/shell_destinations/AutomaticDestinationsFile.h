#pragma once

#include <memory>
#include <vector>
#include <string>

namespace launcherapp { namespace commands { namespace shell_destinations {

struct AutomaticDestinationEntry
{
  std::wstring displayPath;   // 表示名(ReadShellLinkNameStringの内容)
  std::vector<unsigned char> lnk;  // .lnkファイルの内容
  bool isDirectory;          // true:リンク先の実体はディレクトリ false:ディレクトリではない
};


class AutomaticDestinationsFile
{
public:
  /** 管理用のデータを初期化する */
  AutomaticDestinationsFile();

  /** 管理用のデータを破棄する */
  ~AutomaticDestinationsFile();

  /**
    AutomaticDestinationsファイルを開き、ピン留めされた項目を読み込む
    @param[in] filePath 読み込むファイルのパス
    @return true:読み込み成功 false:読み込み失敗
  */
  bool Open(const wchar_t* filePath);

  /**
    読み込み済みのピン留め項目数を取得する
    @return ピン留め項目数
  */
  size_t GetPinnedEntryCount();

  /**
    指定位置のピン留め項目を取得する
    @param[in] index 取得する項目の位置
    @param[out] entry 取得した項目
    @return true:取得成功 false:位置が範囲外
  */
  bool GetPinnedEntry(size_t index, AutomaticDestinationEntry& entry);

private:
  struct PImpl;
  std::unique_ptr<PImpl> in;
};

}}}

