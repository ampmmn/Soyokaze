#pragma once

#include <vector>
#include "actions/core/ActionParameterIF.h"

namespace launcherapp {
namespace commands {
namespace common {

// 実行時引数($1...$n, $*)を展開する
void ExpandArguments(CString& target, const std::vector<CString>& args);
void ExpandArguments(CString& target, const launcherapp::actions::core::Parameter* param);

// ランチャーのマクロ機能による置換
bool ExpandMacros(CString& target);

// 前後のダブルクォーテーションを除去する
void StripDoubleQuate(CString& str);

} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp


