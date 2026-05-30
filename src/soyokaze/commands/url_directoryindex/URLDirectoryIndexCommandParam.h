#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include "commands/core/CommandEntryIF.h"

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

	bool Save(CommandEntryIF* entry);
	bool Load(CommandEntryIF* entry);

	void SetServerPassword(const CString& data);
	CString DecryptServerPassword();
	void SetProxyPassword(const CString& data);
	CString DecryptProxyPassword();

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
	std::vector<uint8_t> mServerPassword;
	// プロキシ認証のユーザ名とパスワード
	int mProxyType;
	CString mProxyHost;
	CString mProxyUser;
	std::vector<uint8_t> mProxyPassword;

	CommandHotKeyAttribute mHotKeyAttr;
};

} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

