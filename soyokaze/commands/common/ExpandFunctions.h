#pragma once

#include <vector>

namespace launcherapp {
namespace commands {
namespace common {

// 実行時引数($1...$n, $*)を展開する
void ExpandArguments(CString& target, const std::vector<CString>& args);
// 環境変数($name)を展開する
void ExpandEnv(CString& target);
// その他の変数を展開する
void ExpandVariable(CString& target, const CString& name, const CString& value);
// クリップボードの文字列で展開する
void ExpandClipboard(CString& target);

// あふwで現在表示しているディレクトリを表示する
bool ExpandAfxCurrentDir(CString& target);


} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp


