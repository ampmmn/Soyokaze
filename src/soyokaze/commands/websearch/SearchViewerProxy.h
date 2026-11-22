#pragma once

#include <cstdint>
#include <memory>

namespace launcherapp {
namespace commands {
namespace websearch {

class SearchViewerProxy
{
public:
	using RegistrationToken = uint64_t;

	/** シングルトンを取得する */
	static SearchViewerProxy* GetInstance();
	/** 検索ウインドウを管理するUIスレッドを登録する */
	void InitializeOnUiThread();

	/** 空タイトルをコマンド名に基づく既定タイトルへ置き換える */
	static CString CreateWindowTitle(const CString& commandName, const CString& title);

	/** 専用検索ウインドウを使うコマンドを登録する */
	RegistrationToken RegisterCommand(const CString& commandName, const CString& title);
	/** 登録トークンが一致するコマンド登録を解除する */
	void UnregisterCommand(const CString& commandName, RegistrationToken token);
	/** 登録済みコマンドに対応するウインドウで検索する */
	bool Open(const CString& commandName, const CString& url);
	/** 全ての検索ウインドウを閉じる */
	void CloseAll();

private:
	SearchViewerProxy();
	~SearchViewerProxy();

	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
