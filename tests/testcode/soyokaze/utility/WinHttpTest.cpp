#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/WinHttp.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm>
#include <cctype>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#pragma comment (lib, "ws2_32.lib")

using launcherapp::WinHttp;
using launcherapp::HttpResponse;

namespace {

/**
  テスト用のローカルHTTPサーバ
  1接続につき1リクエストを受け付け、ハンドラが返した生のレスポンスを送信する
*/
class TestHttpServer
{
public:
	using Handler = std::function<std::string(const std::string& request)>;

	~TestHttpServer()
	{
		Stop();
	}

	/**
	  127.0.0.1の空きポートで待ち受けを開始する
	  @param[in] handler リクエストを受け取りレスポンス文字列を返す関数
	  @return true:成功  false:失敗
	*/
	bool Start(Handler handler)
	{
		// Winsockを初期化する
		WSADATA wsa{};
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
			return false;
		}
		mWsaStarted = true;

		mListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (mListenSocket == INVALID_SOCKET) {
			return false;
		}

		// ポート0で待ち受け、割り当てられたポート番号を取得する
		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		addr.sin_port = 0;
		if (bind(mListenSocket, (sockaddr*)&addr, sizeof(addr)) != 0) {
			return false;
		}

		int len = sizeof(addr);
		if (getsockname(mListenSocket, (sockaddr*)&addr, &len) != 0) {
			return false;
		}
		mPort = ntohs(addr.sin_port);

		if (listen(mListenSocket, 8) != 0) {
			return false;
		}

		mHandler = handler;
		SOCKET listenSock = mListenSocket;
		mThread = std::thread([this, listenSock] { AcceptLoop(listenSock); });
		return true;
	}

	/**
	  待ち受けを停止する
	  closesocketでaccept待ちを解除し、スレッドの終了を待つ
	*/
	void Stop()
	{
		if (mListenSocket != INVALID_SOCKET) {
			closesocket(mListenSocket);
			mListenSocket = INVALID_SOCKET;
		}
		if (mThread.joinable()) {
			mThread.join();
		}
		if (mWsaStarted) {
			WSACleanup();
			mWsaStarted = false;
		}
	}

	int Port() const { return mPort; }

	/**
	  最後に受信した生のリクエストを返す
	*/
	std::string LastRequest() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mLastRequest;
	}

private:
	void AcceptLoop(SOCKET listenSock)
	{
		for (;;) {
			SOCKET client = accept(listenSock, nullptr, nullptr);
			if (client == INVALID_SOCKET) {
				// Stopによりソケットが閉じられた
				break;
			}
			HandleConnection(client);
			closesocket(client);
		}
	}

	void HandleConnection(SOCKET client)
	{
		// リクエストを記録してからハンドラでレスポンスを作る
		std::string request = ReadRequest(client);
		{
			std::lock_guard<std::mutex> lock(mMutex);
			mLastRequest = request;
		}

		std::string response = mHandler(request);
		send(client, response.data(), (int)response.size(), 0);
	}

	/**
	  ヘッダと、Content-Lengthで示されるボディを読み切る
	*/
	static std::string ReadRequest(SOCKET client)
	{
		std::string data;
		char buf[4096];
		size_t headerEnd = std::string::npos;
		size_t contentLength = 0;

		for (;;) {
			int n = recv(client, buf, sizeof(buf), 0);
			if (n <= 0) {
				break;
			}
			data.append(buf, n);

			// ヘッダ終端を見つけたらContent-Lengthを取得する
			if (headerEnd == std::string::npos) {
				headerEnd = data.find("\r\n\r\n");
				if (headerEnd != std::string::npos) {
					contentLength = ParseContentLength(data.substr(0, headerEnd));
				}
			}
			if (headerEnd != std::string::npos && data.size() >= headerEnd + 4 + contentLength) {
				break;
			}
		}
		return data;
	}

	/**
	  ヘッダ文字列からContent-Lengthの値を取得する(無い場合は0)
	*/
	static size_t ParseContentLength(const std::string& header)
	{
		std::string lower = header;
		std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return (char)tolower(c); });

		size_t pos = lower.find("content-length:");
		if (pos == std::string::npos) {
			return 0;
		}
		return (size_t)atoi(lower.c_str() + pos + 15);
	}

	SOCKET mListenSocket{INVALID_SOCKET};
	int mPort{0};
	bool mWsaStarted{false};
	Handler mHandler;
	std::thread mThread;
	mutable std::mutex mMutex;
	std::string mLastRequest;
};

/**
  ステータス行、追加ヘッダ、ボディからHTTPレスポンス文字列を組み立てる
  @param[in] statusLine   例: "200 OK"
  @param[in] extraHeaders 「Name: Value\r\n」形式の追加ヘッダ
  @param[in] body         レスポンスボディ
*/
std::string MakeResponse(const std::string& statusLine, const std::string& extraHeaders, const std::string& body)
{
	std::string res = "HTTP/1.1 " + statusLine + "\r\n";
	res += extraHeaders;
	res += "Content-Length: " + std::to_string(body.size()) + "\r\n";
	res += "Connection: close\r\n\r\n";
	res += body;
	return res;
}

/**
  大文字小文字を区別せずに部分文字列が含まれるかを判定する
*/
bool ContainsIgnoreCase(const std::string& text, const std::string& word)
{
	std::string lowerText = text;
	std::string lowerWord = word;
	std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), [](unsigned char c) { return (char)tolower(c); });
	std::transform(lowerWord.begin(), lowerWord.end(), lowerWord.begin(), [](unsigned char c) { return (char)tolower(c); });
	return lowerText.find(lowerWord) != std::string::npos;
}

/**
  127.0.0.1のポートとパスからURLを組み立てる
*/
CString MakeUrl(int port, const char* path)
{
	CString url;
	url.Format(_T("http://127.0.0.1:%d%hs"), port, path);
	return url;
}

} // namespace


// GETでステータス200とボディ、Content-Typeを取得できること
TEST(WinHttpTest, Request_Get_ReturnsStatusAndBody)
{
	TestHttpServer server;
	ASSERT_TRUE(server.Start([](const std::string&) {
		return MakeResponse("200 OK", "Content-Type: text/plain\r\n", "hello");
	}));

	WinHttp http;
	http.SetProxyType(WinHttp::NOPROXY);
	http.SetMethod(L"GET");

	HttpResponse response;
	ASSERT_TRUE(http.Request(MakeUrl(server.Port(), "/"), "", response));
	EXPECT_EQ(200, response.statusCode);
	EXPECT_EQ("hello", response.body);
	EXPECT_STREQ(_T("text/plain"), (LPCTSTR)response.contentType);
}

// POSTでボディがサーバに届くこと
TEST(WinHttpTest, Request_Post_SendsBody)
{
	TestHttpServer server;
	ASSERT_TRUE(server.Start([](const std::string&) {
		return MakeResponse("200 OK", "", "");
	}));

	const std::string body = "{\"using\":\"accessibility id\",\"value\":\"btnLogin\"}";

	WinHttp http;
	http.SetProxyType(WinHttp::NOPROXY);
	http.SetMethod(L"POST");

	HttpResponse response;
	ASSERT_TRUE(http.Request(MakeUrl(server.Port(), "/session/1/element"), body, response));

	std::string req = server.LastRequest();
	EXPECT_EQ(0, req.find("POST /session/1/element "));
	EXPECT_NE(std::string::npos, req.find(body));
}

// SetContentTypeで指定したContent-Typeがサーバに届くこと
TEST(WinHttpTest, Request_SetContentType_SendsHeader)
{
	TestHttpServer server;
	ASSERT_TRUE(server.Start([](const std::string&) {
		return MakeResponse("200 OK", "", "");
	}));

	WinHttp http;
	http.SetProxyType(WinHttp::NOPROXY);
	http.SetMethod(L"POST");
	http.SetContentType(L"application/json; charset=utf-8");

	HttpResponse response;
	ASSERT_TRUE(http.Request(MakeUrl(server.Port(), "/session"), "{}", response));

	EXPECT_NE(std::string::npos, server.LastRequest().find("Content-Type: application/json; charset=utf-8"));
}

// Content-Typeを設定しない場合は送信されないこと(既存互換)
TEST(WinHttpTest, Request_WithoutContentType_NotSent)
{
	TestHttpServer server;
	ASSERT_TRUE(server.Start([](const std::string&) {
		return MakeResponse("200 OK", "", "");
	}));

	WinHttp http;
	http.SetProxyType(WinHttp::NOPROXY);
	http.SetMethod(L"POST");

	HttpResponse response;
	ASSERT_TRUE(http.Request(MakeUrl(server.Port(), "/session"), "{}", response));

	EXPECT_FALSE(ContainsIgnoreCase(server.LastRequest(), "content-type:"));
}

// 空文字列を設定するとContent-Typeが送信されなくなること
TEST(WinHttpTest, Request_SetContentTypeEmpty_RemovesHeader)
{
	TestHttpServer server;
	ASSERT_TRUE(server.Start([](const std::string&) {
		return MakeResponse("200 OK", "", "");
	}));

	WinHttp http;
	http.SetProxyType(WinHttp::NOPROXY);
	http.SetMethod(L"POST");
	http.SetContentType(L"application/json");
	http.SetContentType(L"");

	HttpResponse response;
	ASSERT_TRUE(http.Request(MakeUrl(server.Port(), "/session"), "{}", response));

	EXPECT_FALSE(ContainsIgnoreCase(server.LastRequest(), "content-type:"));
}

// 404 / 500 でもtrueを返し、ステータスとJSONボディを取得できること
TEST(WinHttpTest, Request_ErrorStatus_ReturnsTrueWithStatus)
{
	TestHttpServer server;
	ASSERT_TRUE(server.Start([](const std::string& req) {
		if (req.find("GET /notfound ") == 0) {
			return MakeResponse("404 Not Found", "Content-Type: application/json; charset=utf-8\r\n",
				"{\"value\":{\"error\":\"no such element\"}}");
		}
		return MakeResponse("500 Internal Server Error", "Content-Type: application/json; charset=utf-8\r\n",
			"{\"value\":{\"error\":\"session not created\"}}");
	}));

	WinHttp http;
	http.SetProxyType(WinHttp::NOPROXY);
	http.SetMethod(L"GET");

	HttpResponse notFound;
	ASSERT_TRUE(http.Request(MakeUrl(server.Port(), "/notfound"), "", notFound));
	EXPECT_EQ(404, notFound.statusCode);
	EXPECT_EQ("{\"value\":{\"error\":\"no such element\"}}", notFound.body);

	HttpResponse serverError;
	ASSERT_TRUE(http.Request(MakeUrl(server.Port(), "/error"), "", serverError));
	EXPECT_EQ(500, serverError.statusCode);
	EXPECT_EQ("{\"value\":{\"error\":\"session not created\"}}", serverError.body);
	EXPECT_STREQ(_T("application/json; charset=utf-8"), (LPCTSTR)serverError.contentType);
}

// 認証が繰り返し要求される場合もtrueとステータス401を返すこと
TEST(WinHttpTest, Request_Unauthorized_ReturnsTrueWith401)
{
	TestHttpServer server;
	ASSERT_TRUE(server.Start([](const std::string&) {
		return MakeResponse("401 Unauthorized", "WWW-Authenticate: Basic realm=\"test\"\r\n", "");
	}));

	WinHttp http;
	http.SetProxyType(WinHttp::NOPROXY);
	http.SetMethod(L"GET");

	HttpResponse response;
	ASSERT_TRUE(http.Request(MakeUrl(server.Port(), "/"), "", response));
	EXPECT_EQ(401, response.statusCode);
}

// 接続できないポートへの通信はfalseを返すこと
TEST(WinHttpTest, Request_ConnectionFailure_ReturnsFalse)
{
	WinHttp http;
	http.SetProxyType(WinHttp::NOPROXY);
	http.SetMethod(L"GET");
	http.SetTimeout(3000);

	// ポート1は通常待ち受けされていないため接続は拒否される
	HttpResponse response;
	EXPECT_FALSE(http.Request(_T("http://127.0.0.1:1/"), "", response));
}

// LoadContentは200以外をfalseとして扱うこと(既存挙動)
TEST(WinHttpTest, LoadContent_NonOkStatus_ReturnsFalse)
{
	TestHttpServer server;
	ASSERT_TRUE(server.Start([](const std::string&) {
		return MakeResponse("404 Not Found", "Content-Type: text/html\r\n", "<html></html>");
	}));

	WinHttp http;
	http.SetProxyType(WinHttp::NOPROXY);

	std::vector<BYTE> content;
	bool isHTML = false;
	EXPECT_FALSE(http.LoadContent(MakeUrl(server.Port(), "/"), content, isHTML));
}

// SetContentTypeを設定してもLoadBinaryContentの送信ヘッダには付かないこと
TEST(WinHttpTest, LoadBinaryContent_IgnoresContentType)
{
	TestHttpServer server;
	ASSERT_TRUE(server.Start([](const std::string&) {
		return MakeResponse("200 OK", "Content-Type: text/html\r\n", "<html></html>");
	}));

	WinHttp http;
	http.SetProxyType(WinHttp::NOPROXY);
	http.SetContentType(L"application/json; charset=utf-8");

	std::vector<BYTE> content;
	EXPECT_TRUE(http.LoadBinaryContent(MakeUrl(server.Port(), "/"), content));
	EXPECT_FALSE(ContainsIgnoreCase(server.LastRequest(), "content-type:"));
}

