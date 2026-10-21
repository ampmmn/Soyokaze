#pragma once

#include <vector>
#include "actions/core/ActionParameterIF.h"

namespace launcherapp {
namespace commands {
namespace common {

// 展開時に値をクォートする方法
enum class ArgumentQuoting {
	// クォートしない(パスや作業ディレクトリなど、コマンドラインとして渡されない場合)
	None,
	// 常にダブルクォートで囲む(パラメータなど、コマンドラインとして渡される場合)
	Always
};

// 実行時引数($1...$n, $*)を展開する
void ExpandArguments(CString& target, const std::vector<CString>& args, ArgumentQuoting quoting = ArgumentQuoting::Always);
void ExpandArguments(CString& target, const launcherapp::actions::core::Parameter* param, ArgumentQuoting quoting = ArgumentQuoting::Always);

// 実行時引数とマクロをまとめて展開する
// 展開された値が再度展開されることはない
// @param[in,out] target 展開する文字列
// @param[in] args 実行時引数
// @param[in] quoting 値のクォート方法
// @param[in] expandMacros マクロを展開するかどうか
void ExpandArgumentsAndMacros(CString& target, const std::vector<CString>& args, ArgumentQuoting quoting, bool expandMacros);
void ExpandArgumentsAndMacros(CString& target, const launcherapp::actions::core::Parameter* param, ArgumentQuoting quoting, bool expandMacros);

// ランチャーのマクロ機能による置換
bool ExpandMacros(CString& target);

// 前後のダブルクォーテーションを除去する
void StripDoubleQuate(CString& str);

} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp

