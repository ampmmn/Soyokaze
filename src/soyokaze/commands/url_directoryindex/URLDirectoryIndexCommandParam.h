#pragma once

#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	CString CombineURL(const CString& subPath) const;
	CString CombineURL(const CString& subPath1, const CString& subPath2) const;

	static CString CombineURL2(const CString& urlPart, const CString& subPath);

	static void SimplifyURL(CString& url);
public:
	CString mName;
	CString mDescription;
	// ベースURL
	CString mURL;

	// サーバ認証のユーザ名とパスワード
	CString mServerUser;
	CString mServerPassword;
	// プロキシ認証のユーザ名とパスワード
	int mProxyType;
	CString mProxyHost;
	CString mProxyUser;
	CString mProxyPassword;

	CommandHotKeyAttribute mHotKeyAttr;
};

} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

