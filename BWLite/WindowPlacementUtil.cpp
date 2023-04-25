#include "pch.h"
#include "framework.h"
#include "WindowPlacementUtil.h"
#include "AppProfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace util {
namespace window {

static LPCTSTR c_pszSection = _T("WindowPlacement");

/*!
 *	@brief ウインドウ位置情報の保存
 *	@return 成功時true
 *	@param[in] wnd 対象ウインドウ
 *	@param[in] key 保存キー
 */
bool SavePlacement(
	CWnd* wnd,
	LPCTSTR key
)
{
	ASSERT(key);
	if (wnd == NULL || wnd->GetSafeHwnd() == NULL) {
		return false;
	}

	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(wnd->GetSafeHwnd(), &wp);
	CAppProfile::Get()->WriteBinary(c_pszSection, key, &wp, wp.length);
	return true;
}

/*!
 *	@brief	ウインドウ位置情報の復元
 *	@return 成功時true
 *	@param[in] wnd 対象ウインドウ
 *	@param[in] key 保存キー
 */
bool LoadPlacement(
	CWnd* wnd,
	LPCTSTR key
)
{
	ASSERT(key);
	if (wnd == NULL || wnd->GetSafeHwnd() == NULL) {
		return false;
	}

	int len = (int)CAppProfile::Get()->GetBinary(c_pszSection, key, NULL, 0);
	if (len == 0) {
		return false;
	}

	WINDOWPLACEMENT wp;
	CAppProfile::Get()->GetBinary(c_pszSection, key, &wp, len);

	return SetWindowPlacement(wnd->GetSafeHwnd(), &wp) != FALSE;
}

}  // end of namespace window
}  // end of namespace util

