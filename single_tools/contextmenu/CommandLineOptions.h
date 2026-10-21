#pragma once

#include <windows.h>

#include <memory>
#include <string>
#include <vector>

/**
  コマンドライン引数を解析し、コンテキストメニュー表示に必要なオプションを保持する
*/
class CommandLineOptions
{
public:
	CommandLineOptions();
	~CommandLineOptions();

	CommandLineOptions(const CommandLineOptions&) = delete;
	CommandLineOptions& operator=(const CommandLineOptions&) = delete;

	/**
	  コマンドライン引数を解析する
	  @param[in] argc 引数の数(プログラム名を含まない)
	  @param[in] argv 引数の配列(プログラム名を含まない)
	*/
	void Parse(int argc, LPWSTR* argv);

	/**
	  解析された対象パスの一覧を取得する
	  @return 対象パスの一覧
	*/
	const std::vector<std::wstring>& GetPaths() const;

	/**
	  表示位置が指定されているかを取得する
	  @return true:指定あり false:指定なし
	*/
	bool HasPosition() const;

	/**
	  指定された表示位置を取得する
	  @return スクリーン座標
	*/
	POINT GetPosition() const;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
