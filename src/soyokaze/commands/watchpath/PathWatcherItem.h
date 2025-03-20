#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace watchpath {

class PathWatcherItem
{
public:
	bool BuildExcludeFilterRegex(std::unique_ptr<tregex>& reg) const;
	static void WildcardToRegexp(const CString& in, std::wstring& out);

	// $B4F;kBP>]%Q%9(B
	CString mPath;
	// $BDLCN%a%C%;!<%8(B
	CString mMessage;
	// $BDLCN$N4V3V(B($BA02s$NDLCN$+$i<!$NDLCN$^$G$N4V3V!"IC?t(B)
	UINT mInterval = 3600;
	// $B=|30%Q%?!<%s(B
	CString mExcludeFilter;
};

}
}
}

