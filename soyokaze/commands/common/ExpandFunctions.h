#pragma once

#include <vector>

namespace soyokaze {
namespace commands {
namespace common {

// 実行時引数($1...$n, $*)を展開する
void ExpandArguments(CString& target, const std::vector<CString>& args);
// 環境変数($name)を展開する
void ExpandEnv(CString& target);
// その他の変数を展開する
void ExpandVariable(CString& target, const CString& name, const CString& value);

} // end of namespace common
} // end of namespace commands
} // end of namespace soyokaze


