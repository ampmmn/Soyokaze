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
  AutomaticDestinationsFile();
  ~AutomaticDestinationsFile();

  bool Open(const wchar_t* filePath);

  size_t GetPinnedEntryCount();
  bool GetPinnedEntry(size_t index, AutomaticDestinationEntry& entry);

private:
  struct PImpl;
  std::unique_ptr<PImpl> in;
};

}}}

