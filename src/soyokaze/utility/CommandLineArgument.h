#pragma once

namespace launcherapp {
namespace utility {

// コマンドライン引数をMSVCの規則(CommandLineToArgvW)に従ってクォートする
// 常にダブルクォートで囲み、埋め込みのダブルクォートは""に、
// ダブルクォートに続く/末尾のバックスラッシュは2倍にして出力する
// (cmd.exe経由の実行でも引用符状態が崩れないため、同じ出力をそのまま利用できる)
CString QuoteArgument(const CString& value);

// QuoteArgumentの、外側のダブルクォートを付けない版
// テンプレート側で既にダブルクォートが付いている場合に利用するため、
// 末尾のバックスラッシュは2倍にする
CString EscapeArgumentValue(const CString& value);

// 値にcmd.exeによって展開される環境変数参照(%NAME%)が含まれるか
// 参照先の環境変数が定義済みの場合にのみtrueを返す
bool ContainsExpandableEnvReference(const CString& value);

// 文字列中のプレースホルダを、値をエスケープしたうえで置換する
// プレースホルダがダブルクォートで囲まれている場合は内側だけエスケープする
// @param[in,out] text 置換する文字列
// @param[in] placeholder 置換対象のプレースホルダ
// @param[in] value 置き換える値
void ReplaceQuotedPlaceholder(CString& text, LPCTSTR placeholder, const CString& value);

}
}
