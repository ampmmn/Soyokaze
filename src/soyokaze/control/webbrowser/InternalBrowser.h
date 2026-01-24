#pragma once

#include <memory>

namespace soyokaze {
namespace control {
namespace webbrowser {

class InternalBrowser : public CWnd
{
	DECLARE_DYNAMIC(InternalBrowser)

public:
	/**
	 * @brief Construct a new Internal Browser object
	 * 
	 */
	InternalBrowser();
	virtual ~InternalBrowser();

	/**
	 * @brief Create window.
	 * 
	 * @param pParentWnd parent window.
	 * @param style window style.
	 * @param rect window rect.
	 * @param nID window ID.
	 * @return TRUE if success, otherwise FALSE.
	 */
	BOOL Create(CWnd* pParentWnd, int style, const RECT& rect, UINT nID);

	void SetHostWindowClass(LPCWSTR clsName);

	void EnableSaveWindowPosition(LPCTSTR settingName);

	/**
	 * @brief Open the specified URL.
	 * 
	 * @param url URL to open.
	 */
	void Open(const CString& url);

	/**
	 * @brief Go back to the previous page.
	 * 
	 */
	void GoBack();

	/**
	 * @brief Go forward to the next page.
	 * 
	 */
	void GoForward();

	/**
	 * @brief Reload the current page.
	 * 
	 */
	void Reload();

protected:
	virtual void PreSubclassWindow();

	void InitializeWebview();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnNcDestroy();

public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // namespace webbrowser
} // namespace control
} // namespace soyokaze

