#pragma once

#include <memory>
#include <string>

class SearchViewerConfig
{
public:
	SearchViewerConfig();
	~SearchViewerConfig();

	SearchViewerConfig(const SearchViewerConfig&) = delete;
	SearchViewerConfig& operator=(const SearchViewerConfig&) = delete;
	SearchViewerConfig(SearchViewerConfig&&) noexcept;
	SearchViewerConfig& operator=(SearchViewerConfig&&) noexcept;

	/** YAML文字列を解析する
	  @param[in] yaml YAML文字列
	  @param[out] result 解析結果
	  @param[out] error エラー内容
	  @return true:成功 false:失敗
	*/
	static bool Parse(const std::string& yaml, SearchViewerConfig& result, std::wstring& error);

	/** 設定ファイルを読み込む
	  @param[in] path 設定ファイルのパス
	  @param[out] result 読み込み結果
	  @param[out] error エラー内容
	  @return true:成功 false:失敗
	*/
	static bool LoadFile(const std::wstring& path, SearchViewerConfig& result, std::wstring& error);

	/** プロファイルの表示名を取得する
	  @param[in] profile プロファイル名
	  @param[out] title 表示名
	  @return true:存在する false:存在しない
	*/
	bool GetTitle(const std::wstring& profile, std::wstring& title) const;

	/** プロファイルと検索語から検索URLを生成する
	  @param[in] profile プロファイル名
	  @param[in] query 検索語
	  @param[out] url 生成したURL
	  @param[out] error エラー内容
	  @return true:成功 false:失敗
	*/
	bool BuildSearchUrl(const std::wstring& profile, const std::wstring& query, std::wstring& url, std::wstring& error) const;

	/** URLがHTTPまたはHTTPSであることを確認する
	  @param[in] url 確認するURL
	  @return true:許可する false:許可しない
	*/
	static bool IsAllowedUrl(const std::wstring& url);

	/** プロファイル名の形式を確認する
	  @param[in] profile プロファイル名
	  @return true:有効 false:無効
	*/
	static bool IsValidProfileName(const std::wstring& profile);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
