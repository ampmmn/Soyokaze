#pragma once

#include <memory>

struct CFolderDialogData;

// Class: フォルダ選択ダイアログ
class CFolderDialog
{
public:
	CFolderDialog(UINT uTitleID, LPCTSTR pszInitialPath = NULL, CWnd *pwndParent = NULL);
	CFolderDialog(LPCTSTR pszTitle = 0, LPCTSTR pszInitialPath = NULL, CWnd *pwndParent = NULL);
	virtual ~CFolderDialog();

	int DoModal();
	CString GetPathName() const;

protected:
	std::unique_ptr<CFolderDialogData> m_pData;

private:
	void Init(LPCTSTR pszTitle, LPCTSTR pszInitialPath, CWnd *pwndParent);
};
