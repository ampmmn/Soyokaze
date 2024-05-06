#pragma once

#include <vector>

namespace launcherapp {
namespace commands {
namespace common {

// 実行時引数($1...$n, $*)を展開する
void ExpandArguments(CString& target, const std::vector<CString>& args);

// ランチャーのマクロ機能による置換
bool ExpandMacros(CString& target);


} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp


