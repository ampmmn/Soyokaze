#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <string>

class SearchViewerWindowPlacement
{
public:
	/** YAMLから指定プロファイルのウインドウ位置を読み込む
	  @param[in] yaml YAML文字列
	  @param[in] profile プロファイル名
	  @param[out] placement ウインドウ位置
	  @return true:復元可能な位置情報がある false:位置情報がないか不正
	*/
	static bool Parse(const std::string& yaml, const std::wstring& profile, WINDOWPLACEMENT& placement);

	/** YAML文字列のプロファイル位置を更新する
	  @param[in] yaml 元のYAML文字列
	  @param[in] profile プロファイル名
	  @param[in] placement 保存するウインドウ位置
	  @return 更新後のYAML文字列。失敗時は空文字列
	*/
	static std::string Update(const std::string& yaml, const std::wstring& profile, const WINDOWPLACEMENT& placement);

	/** WINDOWPLACEMENTの構造体データをBase64へ変換する
	  @param[in] placement 変換するウインドウ位置
	  @return Base64文字列
	*/
	static std::string Encode(const WINDOWPLACEMENT& placement);

	/** Base64文字列をWINDOWPLACEMENTへ復元する
	  @param[in] encoded Base64文字列
	  @param[out] placement 復元したウインドウ位置
	  @return true:成功 false:失敗
	*/
	static bool Decode(const std::string& encoded, WINDOWPLACEMENT& placement);
};
