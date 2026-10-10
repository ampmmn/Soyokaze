# WinHttp 汎用リクエスト機能 (Issue #315)

`launcherapp::WinHttp` (src/soyokaze/utility/WinHttp.h) に任意のHTTPメソッド、リクエストボディ、Content-Typeを指定して送信し、ステータスコードとボディを取得する機能を追加した。WinAppDriverクライアントの実装で利用することを想定しているが、汎用のHTTPクライアントとして利用できる。

## API概要

| メンバ | 説明 |
|---|---|
| `SetMethod(LPCWSTR)` | HTTPメソッドを設定する (既定値 `GET`) |
| `SetContentType(LPCWSTR)` | `Request()` で送信するContent-Typeを設定する。空文字列を渡すと送信しない |
| `Request(url, requestBody, response)` | リクエストを送信し、`HttpResponse` にステータス、Content-Type、ボディを格納する |

`HttpResponse` の構成は次のとおり。

```cpp
struct HttpResponse
{
	int statusCode{0};      // HTTPステータスコード
	CString contentType;    // Content-Typeヘッダの値 (無い場合は空)
	std::string body;       // レスポンスボディ (UTF-8テキストを想定)
};
```

### 戻り値の扱い

- `Request()` は、レスポンスを受信できた場合は非200であっても `true` を返す。呼び出し側は `response.statusCode` で判定する。
- `false` を返すのは次の場合のみ。
  - 接続失敗、DNS解決失敗、タイムアウト
  - WinHTTP APIエラー
  - レスポンス受信失敗
  - プロキシ認証 (407) の失敗

### 状態保持の注意

`SetMethod()` と `SetContentType()` は設定が次の呼び出しにも残る。WinAppDriver用途では同じ設定で連続送信することが多いため、用途が変わるときは明示的に設定し直すこと。

`SetContentType()` は `Request()` のみに適用される。`LoadContent()` / `LoadBinaryContent()` には追加ヘッダは送られない。

### 認証とプロキシ

既存の `SetServerCredential()` (401時のリトライ)、`SetProxyType()` / `SetProxyCredential()` を利用できる。`SetServerCredential()` を設定した場合、401が返されると認証情報を付けて1回だけ再送する。再送後も401の場合は `true` とステータス401を返す。

## 利用例

### JSONのGET

```cpp
WinHttp http;
http.SetMethod(L"GET");

HttpResponse response;
if (http.Request(L"http://127.0.0.1:4723/status", "", response) &&
    response.statusCode == 200)
{
	auto json = nlohmann::json::parse(response.body);
}
```

### JSONのPOST

```cpp
nlohmann::json body =
{
	{"platformName", "Windows"}
};

WinHttp http;
http.SetMethod(L"POST");
http.SetContentType(L"application/json; charset=utf-8");

HttpResponse response;
if (http.Request(L"http://127.0.0.1:4723/session", body.dump(), response)) {
	// statusCodeで成否を判定する
	if (response.statusCode == 200) {
		auto json = nlohmann::json::parse(response.body);
	}
}
```

## WinAppDriver利用例

WinAppDriverはWebDriverプロトコルに従い、JSON over HTTPで通信する。以下はサーバ既定のURL `http://127.0.0.1:4723` を前提とする。

### セッション作成

```cpp
nlohmann::json body =
{
	{
		"capabilities",
		{
			{
				"alwaysMatch",
				{
					{"platformName", "Windows"},
					{"app", "C:\\Program Files\\MyApp\\MyApp.exe"}
				}
			}
		}
	}
};

WinHttp http;
http.SetMethod(L"POST");
http.SetContentType(L"application/json; charset=utf-8");

HttpResponse response;
if (http.Request(L"http://127.0.0.1:4723/session", body.dump(), response) &&
    response.statusCode == 200)
{
	auto json = nlohmann::json::parse(response.body);
	// 返却値 value.sessionId をセッションIDとして利用する
	std::string sessionId = json["value"]["sessionId"];
}
```

### 要素検索

```cpp
nlohmann::json body =
{
	{"using", "accessibility id"},
	{"value", "btnLogin"}
};

http.SetMethod(L"POST");
http.SetContentType(L"application/json; charset=utf-8");

HttpResponse response;
CString url;
url.Format(_T("http://127.0.0.1:4723/session/%hs/element"), sessionId.c_str());
if (http.Request(url, body.dump(), response)) {
	if (response.statusCode == 200) {
		auto json = nlohmann::json::parse(response.body);
		// 返却値 value の下に要素IDが格納される
	}
	else {
		// 404 (no such element) などのアプリケーションエラー
		auto json = nlohmann::json::parse(response.body);
		std::string error = json["value"]["error"];
	}
}
```

### 要素クリック

```cpp
http.SetMethod(L"POST");
http.SetContentType(L"application/json; charset=utf-8");

HttpResponse response;
CString url;
url.Format(_T("http://127.0.0.1:4723/session/%hs/element/%hs/click"), sessionId.c_str(), elementId.c_str());
http.Request(url, "{}", response);
```

### テキスト入力

```cpp
nlohmann::json body =
{
	{"text", "hello"}
};

http.SetMethod(L"POST");
http.SetContentType(L"application/json; charset=utf-8");

HttpResponse response;
CString url;
url.Format(_T("http://127.0.0.1:4723/session/%hs/element/%hs/value"), sessionId.c_str(), elementId.c_str());
http.Request(url, body.dump(), response);
```

### セッション終了

```cpp
http.SetMethod(L"DELETE");

HttpResponse response;
CString url;
url.Format(_T("http://127.0.0.1:4723/session/%hs"), sessionId.c_str());
http.Request(url, "", response);
```

## 単体テスト

`tests/testcode/soyokaze/utility/WinHttpTest.cpp` にテストを用意している。テストはWinsockで127.0.0.1にローカルHTTPサーバを立てて実行するため、WinAppDriverは不要である。

- GETでステータスとボディ、Content-Typeを取得できる
- POSTのボディがサーバに届く
- `SetContentType()` の値が送信される / 未設定時は送信されない
- 404 / 500 でも `true` を返し、ステータスとボディを取得できる
- 401が繰り返されても `true` とステータス401を返す
- 接続失敗は `false` を返す
- `LoadContent()` は200以外を `false` として扱う (既存挙動)

WinAppDriverとの実通信は単体テストの対象外であり、上記の利用例で手動確認する。

