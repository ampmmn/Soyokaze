#include "pch.h"
#include "framework.h"
#include "AppSettingExtraActionForExplorePath.h"
#include "commands/explorepath/ExtraActionDialog.h"
#include "commands/explorepath/ExplorePathExtraActionSettings.h"
#include "setting/Settings.h"
#include "utility/Path.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp { namespace commands { namespace explorepath {

using Entry = ExtraActionSettings::Entry;

// 
class ExtraActionSettingDialog : public CDialog
{
public:
	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

	bool UpdateStatus();

	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

	CListCtrl* GetAtionListWnd() {
		ASSERT(mListActions);
		return mListActions;
	}

	void AppendEntry(const Entry& entry);
	void SetEntry(int index, const Entry& entry);
	void SwapEntries(int srcIndex, int dstIndex);


// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonUp();
	afx_msg void OnButtonDown();
	afx_msg void OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyItemDblClk(NMHDR *pNMHDR, LRESULT *pResult);

public:
	CListCtrl* mListActions{nullptr};
	std::vector<Entry> mEntries;
	Settings* mSettingsPtr{nullptr};
};

void ExtraActionSettingDialog::AppendEntry(const Entry& entry)
{
	int index = mListActions->GetItemCount();

	ASSERT(mListActions);
	int inserted_pos = mListActions->InsertItem(index, _T(""));
	mEntries.push_back(entry);

	SetEntry(inserted_pos, entry);
}

void ExtraActionSettingDialog::SetEntry(int index, const Entry& entry)
{
	mListActions->SetItemText(index, 0, entry.mLabel);
	mListActions->SetItemText(index, 1, entry.mCommand);
	mListActions->SetItemText(index, 2, entry.mHotkeyAttr.ToString());

	CString targetText;
	if (entry.mIsForFile && entry.mIsForFolder) {
		targetText = _T("ファイルとフォルダ");
	}
	else if (entry.mIsForFolder == false) {
		targetText  = _T("ファイルのみ");
	}
	else {
		targetText  = _T("フォルダのみ");
	}
	mListActions->SetItemText(index, 3, targetText);

	mEntries[index] = entry;
}

void ExtraActionSettingDialog::SwapEntries(int srcIndex, int dstIndex)
{
	auto listActions = GetAtionListWnd();

	// リストコントロール上のテキストを入れ替える
	constexpr int cols = 4;
	for (int col = 0; col < cols; ++col) {
		CString srcText = listActions->GetItemText(srcIndex, col);
		CString dstText = listActions->GetItemText(dstIndex, col);
		listActions->SetItemText(srcIndex, col, dstText);
		listActions->SetItemText(dstIndex, col, srcText);
	}

	// 選択状態を変更
	listActions->SetItemState(srcIndex, 0, LVIS_SELECTED);
	listActions->SetItemState(dstIndex, LVIS_SELECTED, LVIS_SELECTED);

	// データを交換
	std::swap(mEntries[srcIndex], mEntries[dstIndex]);
}

bool ExtraActionSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	return true;
}

bool ExtraActionSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void ExtraActionSettingDialog::OnOK()
{
	auto settingsPtr = mSettingsPtr;

	ExtraActionSettings extraActionSettings;
	extraActionSettings.SwapEntries(mEntries);
	extraActionSettings.Save(settingsPtr);

	__super::OnOK();
}

void ExtraActionSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(ExtraActionSettingDialog, CDialog)
	ON_COMMAND(IDC_BUTTON_ADD, OnButtonAdd)
	ON_COMMAND(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_COMMAND(IDC_BUTTON_UP, OnButtonUp)
	ON_COMMAND(IDC_BUTTON_DOWN, OnButtonDown)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ACTIONS, OnNotifyItemChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ACTIONS, OnNotifyItemDblClk)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL ExtraActionSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	mListActions = (CListCtrl*)GetDlgItem(IDC_LIST_ACTIONS);

	auto listActions = GetAtionListWnd();
	listActions->SetExtendedStyle(listActions->GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	CString strHeader;

	// リストの列を設定する
	LVCOLUMN lvc;
	memset(&lvc,0,sizeof(LV_COLUMN));
	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;

	// ToDo: 列幅を微調整
	strHeader= _T("ラベル");
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 80;
	lvc.fmt = LVCFMT_LEFT;
	listActions->InsertColumn(0, &lvc);
	
	strHeader= _T("コマンド");
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 80;
	lvc.fmt = LVCFMT_LEFT;
	listActions->InsertColumn(1, &lvc);

	strHeader= _T("ホットキー");
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 80;
	lvc.fmt = LVCFMT_LEFT;
	listActions->InsertColumn(2, &lvc);

	strHeader= _T("対象");
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 80;
	lvc.fmt = LVCFMT_LEFT;
	listActions->InsertColumn(3, &lvc);

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool ExtraActionSettingDialog::UpdateStatus()
{
	auto listActions = GetAtionListWnd();

	POSITION pos = listActions->GetFirstSelectedItemPosition();
	bool hasSelect = pos != NULL;

	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(hasSelect);
	GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow(hasSelect);
	if (hasSelect) {
		int itemIndex = listActions->GetNextSelectedItem(pos);
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(itemIndex > 0);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(itemIndex < listActions->GetItemCount()-1);
	}
	else {
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(FALSE);
	}

	return true;
}

void ExtraActionSettingDialog::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;
	auto listActions = GetAtionListWnd();
	listActions->DeleteAllItems();

	ExtraActionSettings settings;
	settings.Load();

	int num_entries = settings.GetEntryCount();
	for (int index = 0; index < num_entries; ++index) {

		Entry entry;
		if (settings.GetEntry(index, entry) == false) {
			continue;
		}

		AppendEntry(entry);
	}
}

void ExtraActionSettingDialog::OnButtonAdd()
{
	// 登録ダイアログを表示し、リストに追加
	ExtraActionDialog dlg;
	if (dlg.DoModal() != IDOK) {
		return ;
	}
	AppendEntry(dlg.GetEntry());
}

void ExtraActionSettingDialog::OnButtonEdit()
{
	// 登録ダイアログを表示し、リストを更新
	auto listActions = GetAtionListWnd();
	POSITION pos = listActions->GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		return ;
	}
	int itemIndex = listActions->GetNextSelectedItem(pos);

	ExtraActionDialog dlg;
	dlg.SetEntry(mEntries[itemIndex]);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	SetEntry(itemIndex, dlg.GetEntry());
}

void ExtraActionSettingDialog::OnButtonDelete()
{
	// 項目を削除
	auto listActions = GetAtionListWnd();
	POSITION pos = listActions->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = listActions->GetNextSelectedItem(pos);
	listActions->DeleteItem(itemIndex);
	mEntries.erase(mEntries.begin() + itemIndex);

	if (itemIndex < listActions->GetItemCount()) {
		listActions->SetItemState(itemIndex, LVIS_SELECTED, LVIS_SELECTED);
	}
	else {
		itemIndex--;
		if (0 <= itemIndex && itemIndex < listActions->GetItemCount()) {
			listActions->SetItemState(itemIndex, LVIS_SELECTED, LVIS_SELECTED);
		}
	}

	UpdateStatus();
	UpdateData(FALSE);
}

void ExtraActionSettingDialog::OnButtonUp()
{
	auto listActions = GetAtionListWnd();
	POSITION pos = listActions->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = listActions->GetNextSelectedItem(pos);
	if (itemIndex == 0) {
		return;
	}
	SwapEntries(itemIndex, itemIndex-1);

	UpdateStatus();
	UpdateData(FALSE);
}

void ExtraActionSettingDialog::OnButtonDown()
{
	auto listActions = GetAtionListWnd();
	POSITION pos = listActions->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = listActions->GetNextSelectedItem(pos);
	if (itemIndex == listActions->GetItemCount()-1) {
		return;
	}
	SwapEntries(itemIndex, itemIndex+1);

	UpdateStatus();
	UpdateData(FALSE);
}

void ExtraActionSettingDialog::OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	UpdateStatus();
	UpdateData(FALSE);
	*pResult = 0;
}

void ExtraActionSettingDialog::OnNotifyItemDblClk(
		NMHDR *pNMHDR,
	 	LRESULT *pResult
)
{
	*pResult = 0;

	NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;

	int index = nm->iItem;
	if (index < 0 || (int)mEntries.size() <= index) {
		return;
	}

	// 項目の編集
	ExtraActionDialog dlg;
	dlg.SetEntry(mEntries[index]);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	SetEntry(index, dlg.GetEntry());
}

}}}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

using namespace launcherapp::commands::explorepath;

struct AppSettingExtraActionForExplorePath::PImpl
{
	ExtraActionSettingDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingExtraActionForExplorePath)

AppSettingExtraActionForExplorePath::AppSettingExtraActionForExplorePath() : 
	AppSettingPageBase(_T("実行"), _T("ファイルとフォルダ")),
	in(new PImpl)
{
}

AppSettingExtraActionForExplorePath::~AppSettingExtraActionForExplorePath()
{
}

// ウインドウを作成する
bool AppSettingExtraActionForExplorePath::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_EXTRAACTION_FOR_EXPLOREPATH, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingExtraActionForExplorePath::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingExtraActionForExplorePath::GetOrder()
{
	return 210;
}
// 
bool AppSettingExtraActionForExplorePath::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingExtraActionForExplorePath::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingExtraActionForExplorePath::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingExtraActionForExplorePath::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingExtraActionForExplorePath::GetHelpPageId(String& id)
{
	id = "ExtraActionForExplorePathSetting";
	return true;
}

