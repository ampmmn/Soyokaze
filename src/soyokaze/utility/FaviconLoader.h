#pragma once

#include <memory>
#include <vector>
#include <utility/winhttp.h>

namespace launcherapp {

class FaviconLoader
{
public:
	FaviconLoader();
	~FaviconLoader();

	void SetProxyType(int type);
	void SetProxyCredential(const CString& host, const CString& user, const CString& password);
	void SetProxy(const CString& host, WORD port, const CString& user, const CString& password);

	// The returned icon is owned by the caller and must be released with DestroyIcon.
	HICON Load(const CString& url);
	bool Load(const CString& url, std::vector<BYTE>& image);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace launcherapp
