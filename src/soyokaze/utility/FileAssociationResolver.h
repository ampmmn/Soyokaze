#pragma once

namespace launcherapp {
namespace utility {

class FileAssociationResolver
{
public:
	/**
	  ファイルの関連付けに基づき、起動するプログラムと引数を解決する
	  @param[in]  filePath       実行対象ファイル
	  @param[in]  arguments      実行時引数
	  @param[out] executablePath 起動するプログラムのパス
	  @param[out] commandLine    起動するプログラムに渡す引数
	  @return 解決できた場合はtrue
	*/
	bool Resolve(const CString& filePath, const CString& arguments, CString& executablePath, CString& commandLine) const;

	/**
	  関連付けコマンドを実行ファイルと引数に分割する
	  @param[in]  command       関連付けコマンド
	  @param[out] executablePath 実行ファイルのパス
	  @param[out] arguments     引数
	  @return 分割できた場合はtrue
	*/
	static bool ParseAssociationCommand(const CString& command, CString& executablePath, CString& arguments);

private:
	/**
	  指定拡張子の既定のopenコマンドを取得する
	  @param[in] extension ファイル拡張子
	  @param[out] command 関連付けコマンド
	  @return 取得できた場合はtrue
	*/
	static bool QueryAssociationCommand(const CString& extension, CString& command);
	/**
	  関連付けコマンド内のプレースホルダーを置換する
	  @param[in,out] text 関連付けコマンドの引数
	  @param[in] placeholder 置換対象
	  @param[in] value 置換後の値
	  @param[in] quoteValue 値を引用符で囲むか
	*/
	static void ReplacePlaceholder(CString& text, LPCTSTR placeholder, const CString& value, bool quoteValue);
};

}
}
