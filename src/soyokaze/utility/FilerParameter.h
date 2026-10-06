#pragma once

namespace launcherapp { namespace utility {

/**
  ファイラー起動時の引数に対象パスを展開する
  @param[in,out] parameter ファイラーに渡す引数
  @param[in] targetPath 展開する対象パス
*/
void ExpandFilerParameter(CString& parameter, const CString& targetPath);

}}
