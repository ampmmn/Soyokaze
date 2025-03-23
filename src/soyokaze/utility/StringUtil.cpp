#include "pch.h"
#include "StringUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace utility {

CString GetFirstLine(const CString& str)
{
	int pos = 0;
	pos	= str.Find(_T("\r\n"));
	if (pos == -1) {
		pos	= str.Find(_T("\n"));
		if (pos == -1) {
			return str;
		}
	}
	return str.Left(pos);
}

}
}

