#pragma once

#include <memory>
#include <vector>

namespace launcherapp {

class WinHttp
{
public:
	WinHttp();
	~WinHttp();

	bool LoadContent(const CString& url, std::vector<BYTE>& content, bool& isHTML);

	enum {
		SYSTEMSETTING = 0,  // システム設定を使う
		DIRECTPROXY,        // プロキシ指定する
		NOPROXY,            // プロキシを使用しない
	};

	void SetProxyType(int type);
	void SetProxyCredential(const CString& host, const CString& user, const CString& password);
	void SetServerCredential(const CString& user, const CString& password);


private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

} // end of namespace launcherapp

