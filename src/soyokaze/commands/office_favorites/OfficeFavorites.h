#pragma once

#include <memory>
#include <string>
#include <vector>

namespace launcherapp {
namespace commands {
namespace office_favorites {

/**
 * @brief Office のアプリ種別
 * @note 1バイトで保持するため uint8_t を基底とする
 */
enum class OfficeAppType : uint8_t
{
	Word = 0,
	Excel = 1,
	PowerPoint = 2,
};

/**
 * @brief Office のお気に入り(ピン止め)1件分の情報
 * @note mPath と mAppType は必ず保持する。mName は JSON の title から組み立てた表示名で、
 *       空の場合はパスから求める。URL判定や拡張子は都度求める
 */
struct OfficeFavoriteItem
{
	// ローカルパスまたはURL
	CString mPath;
	// アプリ種別
	OfficeAppType mAppType{OfficeAppType::Word};
	// 表示名(JSON の title と extension から組み立てたもの。無い場合は空)
	CString mName;

	// URLか(http:// または https:// で始まるか。true: SharePoint/OneDrive上のファイル)
	bool IsURL() const;
	// 表示名(mName があればそれを、なければパスの末尾要素を返す。URLはデコード済み)
	CString GetName() const;
	// 拡張子(ドット付き。例: ".xlsx"。取得できない場合は空)
	CString GetExtension() const;
	// アプリ種別を取得する
	OfficeAppType GetAppType() const { return mAppType; }
};

/**
 * @brief Office のお気に入りを保持し、問い合わせに応じて返すクラス
 * @note レジストリからの取得と JSON からの取得は独立して行い、それぞれ個別に再読み込みできる。
 *       再読み込みは更新内容を別に構築してから swap するため、保護は swap の瞬間だけ行う。
 */
class OfficeFavorites
{
public:
	OfficeFavorites();
	~OfficeFavorites();

	// レジストリ(ローカルファイル)の情報を再読み込みする
	void ReloadRegistry();
	// JSON(SharePoint/OneDrive)の情報を再読み込みする
	void ReloadJson();
	// 保持している情報をすべて破棄する
	void Clear();

	// 保持しているお気に入りの一覧を取得する
	std::vector<OfficeFavoriteItem> GetItems();

public:
	/**
	 * @brief レジストリの File MRU の値からピン止めされたパスを取り出す
	 * @param[in]  value レジストリ値(例: [F00000001][T01DD...][O00000000]*C:\path\file.xlsx)
	 * @param[out] path  ピン止めされている場合はパス
	 * @return true:ピン止めされている  false:ピン止めされていない、または形式不正
	 */
	static bool ParseRegistryValue(const CString& value, CString& path);

	/**
	 * @brief aggmru の JSON 文字列からピン止めされた項目を取り出す
	 * @param[in]  text  JSON文字列(UTF-8)
	 * @param[out] items 抽出した項目を追加する
	 * @return true:解析成功  false:解析失敗
	 */
	static bool ParseAggMruJson(const std::string& text, std::vector<OfficeFavoriteItem>& items);

	/**
	 * @brief aggmru 配下の対象 JSON ファイル(x/w/p-mru4-*-sr.json)を列挙する
	 * @param[out] files 見つかったファイルのパス
	 */
	static void EnumAggMruJsonFiles(std::vector<CString>& files);

	/**
	 * @brief アプリ種別に対応する実行ファイルのパスを App Paths から取得する
	 * @param[in]  appType アプリ種別
	 * @param[out] exePath 実行ファイルのパス
	 * @return true:取得成功  false:失敗
	 */
	static bool GetAppExePath(OfficeAppType appType, CString& exePath);

	/**
	 * @brief アプリ名("Excel" "Word" "PowerPoint")からアプリ種別を取得する
	 * @param[in]  appName アプリ名
	 * @param[out] appType アプリ種別
	 * @return true:対応するアプリ  false:対象外
	 */
	static bool AppTypeFromName(const CString& appName, OfficeAppType& appType);

	/**
	 * @brief アプリ種別に対応するアプリ名を取得する
	 * @param[in] appType アプリ種別
	 * @return アプリ名("Excel" "Word" "PowerPoint")
	 */
	static CString AppTypeName(OfficeAppType appType);

	// レジストリで監視対象となる User MRU のキーパス(アプリごと)を取得する
	static void GetUserMRUKeyPaths(std::vector<CString>& keyPaths);

	/**
	 * @brief URL をパーセントデコードする(UTF-8 として解釈する)
	 * @param[in] url デコード対象のURL
	 * @return デコード後の文字列。失敗時は元の文字列
	 */
	static CString DecodeUrl(const CString& url);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace office_favorites
} // end of namespace commands
} // end of namespace launcherapp
