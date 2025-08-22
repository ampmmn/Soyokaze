#pragma once

namespace launcherapp { namespace commands { namespace winscp {

// WinSCPのセッション情報にアクセスするためのインタフェース
class SessionStrage
{
public:
	virtual ~SessionStrage() {}

	// 設定情報に更新があったかどうかを取得する
	virtual bool HasUpdate() = 0;
	// 設定情報を取得する
	virtual bool LoadSessions(std::vector<CString>& sessionNames) = 0;

};

}}} // end of namespace launcherapp::commands::winscp

