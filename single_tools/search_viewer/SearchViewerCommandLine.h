#pragma once

#include <string>
#include <vector>

class SearchViewerCommandLine
{
public:
	struct Options
	{
		std::vector<std::wstring> profiles;
		std::wstring configPath;
		std::wstring query;
	};

	/** コマンドライン引数を解析する
	  @param[in] arguments 実行ファイル名を除いた引数
	  @param[out] options 解析結果
	  @param[out] error エラー内容
	  @return true:成功 false:失敗
	*/
	static bool Parse(const std::vector<std::wstring>& arguments, Options& options, std::wstring& error);

	/** Windowsコマンドライン用に引数を引用する
	  @param[in] argument 引用する引数
	  @return 引用済み引数
	*/
	static std::wstring QuoteArgument(const std::wstring& argument);
};
