#pragma once

#include <map>
#include <string>

/**
  ウインドウ位置情報のYAML表現を扱うクラス
  ウインドウ種別ごとに独立した位置情報を保持するため、
  YAMLは「ウインドウ名 -> (モニター構成識別子 -> WINDOWPLACEMENT)」の2階層マップで表す
*/
class WindowPlacementYaml
{
public:
	/** モニター構成識別子とウインドウ位置の対応 */
	using PlacementMap = std::map<std::wstring, WINDOWPLACEMENT>;
	/** ウインドウ名ごとの位置情報の対応 */
	using WindowPlacementMap = std::map<std::wstring, PlacementMap>;

	WindowPlacementYaml();
	~WindowPlacementYaml();

public:
	/**
	  YAML文字列から位置情報を読み込む
	  値がマップでないエントリは旧形式のフラット形式とみなし、メインウインドウの名前で扱う
	  @return true:読み込んだ false:読み込めなかった(または引数が不正)
	  @param[in] yaml 読み込むYAML文字列
	  @param[out] result 読み込んだ位置情報
	*/
	static bool Parse(const std::string& yaml, WindowPlacementMap& result);

	/**
	  位置情報をYAML文字列へ変換する
	  キーに使用できない文字を含むウインドウがある場合は空文字列を返す
	  @return YAML文字列。変換できなかった場合は空文字列
	  @param[in] placements 変換する位置情報
	*/
	static std::string Emit(const WindowPlacementMap& placements);

	/**
	  ウインドウ名をYAMLのキーとして使用できるか確認する
	  @return true:使用できる false:使用できない
	  @param[in] name ウインドウ名
	*/
	static bool IsValidWindowName(const std::wstring& name);

	/** メインウインドウ(旧形式のフラット形式からの移行先)の名前 */
	static const wchar_t* GetMainWindowName();
};