#pragma once

#include <memory>
#include <string>
#include <vector>

namespace launcherapp {

/**
  HTTPレスポンスの情報を保持する構造体
*/
struct HttpResponse
{
	int statusCode{0};       // HTTPステータスコード
	CString contentType;     // Content-Typeヘッダの値(無い場合は空)
	std::string body;        // レスポンスボディ(UTF-8などのテキストを想定)
};

class WinHttp
{
public:
	WinHttp();
	~WinHttp();

	bool LoadContent(const CString& url, std::vector<BYTE>& content, bool& isHTML);
	/**
	  HTTPコンテンツを取得し、リダイレクト後の最終URLも返す
	  @param[in]  url         リクエスト先URL
	  @param[out] content     取得したコンテンツ
	  @param[out] isHTML      HTMLかどうか
	  @param[out] finalURL    リダイレクト後の最終URL
	  @return true:取得成功  false:取得失敗
	*/
	bool LoadContent(const CString& url, std::vector<BYTE>& content, bool& isHTML, CString& finalURL);
	bool LoadBinaryContent(const CString& url, std::vector<BYTE>& content);

	/**
	  任意のHTTPリクエストを送信し、レスポンスを取得する
	  メソッドはSetMethod()、Content-TypeはSetContentType()で事前に設定する
	  HTTPステータスコードが200以外であってもレスポンスを受信できれば true を返す。
	  呼び出し側は response.statusCode で判定すること
	  @param[in]  url         リクエスト先URL
	  @param[in]  requestBody リクエストボディ(空の場合はボディを送らない)
	  @param[out] response    レスポンス(ステータスコード、Content-Type、ボディ)
	  @return true:通信成功(非200を含む)  false:接続失敗、DNS失敗、タイムアウト、APIエラー、受信失敗
	*/
	bool Request(const CString& url, const std::string& requestBody, HttpResponse& response);

	enum {
		SYSTEMSETTING = 0,  // システム設定を使う
		DIRECTPROXY,        // プロキシ指定する
		NOPROXY,            // プロキシを使用しない
	};

	void SetProxyType(int type);
	void SetProxyCredential(const CString& host, const CString& user, const CString& password);
	void SetServerCredential(const CString& user, const CString& password);
	void SetMethod(LPCWSTR method);
	/**
	  Request()で送信するContent-Typeヘッダを設定する
	  設定は次の呼び出しにも残る。LoadContent系には適用されない
	  @param[in] contentType Content-Typeの値(例: application/json; charset=utf-8)
	*/
	void SetContentType(LPCWSTR contentType);
	/**
	  通信タイムアウト時間を設定する
	  @param[in] timeoutMilliseconds タイムアウト時間(ミリ秒)。-1の場合はWinHTTPの既定値を使用する
	*/
	void SetTimeout(int timeoutMilliseconds);


private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

namespace utility {

/**
  2つのURLが同一オリジンか判定する
  相対URLは基準URLと同一オリジンとして扱う
  @param[in] baseURL   基準となる絶対URL
  @param[in] targetURL 判定対象URL
  @return true:同一オリジン  false:異なるオリジンまたは不正なURL
*/
bool IsSameOriginURL(const CString& baseURL, const CString& targetURL);

}

} // end of namespace launcherapp

