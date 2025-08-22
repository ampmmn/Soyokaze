#pragma once

namespace launcherapp { namespace commands { namespace winscp {

// WinSCP$B$N%;%C%7%g%s>pJs$K%"%/%;%9$9$k$?$a$N%$%s%?%U%'!<%9(B
class SessionStrage
{
public:
	virtual ~SessionStrage() {}

	// $B@_Dj>pJs$K99?7$,$"$C$?$+$I$&$+$r<hF@$9$k(B
	virtual bool HasUpdate() = 0;
	// $B@_Dj>pJs$r<hF@$9$k(B
	virtual bool LoadSessions(std::vector<CString>& sessionNames) = 0;

};

}}} // end of namespace launcherapp::commands::winscp

