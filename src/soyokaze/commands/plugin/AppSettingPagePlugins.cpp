#include "pch.h"
#include "framework.h"
#include "AppSettingPagePlugins.h"
#include "PluginProvider.h"
#include "PluginSettings.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace plugin {

class PriorityDialog : public CDialogEx
{
public:
	PriorityDialog(CWnd* parent) : CDialogEx(IDD_PRIORITY, parent) {}
	int mPriority{0};

	void DoDataExchange(CDataExchange* dataExchange) override
	{
		__super::DoDataExchange(dataExchange);
		DDX_Text(dataExchange, IDC_EDIT_PRIORITY, mPriority);
		DDV_MinMaxInt(dataExchange, mPriority, 0, 10000);
	}

	void OnOK() override
	{
		if (UpdateData() != FALSE) {
			__super::OnOK();
		}
	}
};

class PluginListDialog : public CDialog
{
public:
	PluginListDialog() : CDialog(IDD_APPSETTING_PLUGINS) {}

	void SetSettings(PluginSettings* settings) { mSettings = settings; }
	bool OnSetActive();
	bool OnKillActive();
	void OnOK() override;
	BOOL OnInitDialog() override;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonEdit();
	afx_msg void OnRightClick(NMHDR* header, LRESULT* result);
	afx_msg void OnItemChanged(NMHDR* header, LRESULT* result);
	afx_msg void OnGetDispInfo(NMHDR* header, LRESULT* result);

	CListCtrl mListCtrl;
	PluginSettings* mSettings{nullptr};
	std::vector<PluginModulePtr> mPlugins;
	int mSelected{-1};
};

BEGIN_MESSAGE_MAP(PluginListDialog, CDialog)
	ON_COMMAND(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PLUGINS, OnRightClick)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PLUGINS, OnItemChanged)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_PLUGINS, OnGetDispInfo)
END_MESSAGE_MAP()

BOOL PluginListDialog::OnInitDialog()
{
	__super::OnInitDialog();
	mListCtrl.SubclassDlgItem(IDC_LIST_PLUGINS, this);
	mListCtrl.SetExtendedStyle(mListCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	const std::pair<LPCTSTR, int> columns[] = {
		{_T("有効"), 45}, {_T("プラグイン名"), 100}, {_T("優先度"), 50},
		{_T("バージョン"), 65}, {_T("説明"), 190}
	};
	for (int i = 0; i < ARRAYSIZE(columns); ++i) {
		LVCOLUMN column{};
		column.mask = LVCF_TEXT | LVCF_WIDTH;
		column.pszText = const_cast<LPTSTR>(columns[i].first);
		column.cx = columns[i].second;
		mListCtrl.InsertColumn(i, &column);
	}

	if (auto provider = PluginProvider::GetInstance()) {
		provider->EnumPlugins(mPlugins);
	}
	mListCtrl.SetItemCountEx(static_cast<int>(mPlugins.size()));
	UpdateData(FALSE);
	return TRUE;
}

bool PluginListDialog::OnSetActive()
{
	if (mListCtrl.GetSafeHwnd()) {
		mListCtrl.Invalidate();
	}
	return true;
}

bool PluginListDialog::OnKillActive()
{
	return true;
}

void PluginListDialog::OnOK()
{
	if (mSettings) {
		mSettings->CopyTo(PluginSettings::GetInstance());
		PluginSettings::GetInstance()->Save();
	}
	__super::OnOK();
}

void PluginListDialog::OnItemChanged(NMHDR* header, LRESULT* result)
{
	UNREFERENCED_PARAMETER(header);
	*result = 0;
	POSITION position = mListCtrl.GetFirstSelectedItemPosition();
	mSelected = position ? mListCtrl.GetNextSelectedItem(position) : -1;
	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(mSelected >= 0);
}

void PluginListDialog::OnButtonEdit()
{
	if (mSelected < 0 || mSelected >= static_cast<int>(mPlugins.size()) || mSettings == nullptr) {
		return;
	}
	PluginSettings::Item item = mSettings->Get(mPlugins[mSelected]->GetId());
	PriorityDialog dialog(this);
	dialog.mPriority = item.mPriority;
	if (dialog.DoModal() != IDOK) {
		return;
	}
	item.mPriority = dialog.mPriority;
	mSettings->Set(mPlugins[mSelected]->GetId(), item);
	mListCtrl.Invalidate();
}

void PluginListDialog::OnRightClick(NMHDR* header, LRESULT* result)
{
	UNREFERENCED_PARAMETER(header);
	*result = 0;
	if (mSelected < 0 || mSelected >= static_cast<int>(mPlugins.size()) || mSettings == nullptr) {
		return;
	}
	PluginSettings::Item item = mSettings->Get(mPlugins[mSelected]->GetId());
	item.mIsEnabled = !item.mIsEnabled;
	mSettings->Set(mPlugins[mSelected]->GetId(), item);
	mListCtrl.Invalidate();
}

void PluginListDialog::OnGetDispInfo(NMHDR* header, LRESULT* result)
{
	*result = 0;
	NMLVDISPINFO* displayInfo = reinterpret_cast<NMLVDISPINFO*>(header);
	if ((displayInfo->item.mask & LVIF_TEXT) == 0 || displayInfo->item.iItem < 0 ||
		displayInfo->item.iItem >= static_cast<int>(mPlugins.size()) || mSettings == nullptr) {
		return;
	}
	const auto& plugin = mPlugins[displayInfo->item.iItem];
	PluginSettings::Item item = mSettings->Get(plugin->GetId());
	CString value;
	switch (displayInfo->item.iSubItem) {
	case 0: value = item.mIsEnabled ? _T("✔") : _T(""); break;
	case 1: value = plugin->GetDisplayName(); break;
	case 2: value.Format(_T("%d"), item.mPriority); break;
	case 3: value = plugin->GetVersion(); break;
	case 4: value = plugin->GetDescription(); break;
	default: break;
	}
	_tcsncpy_s(displayInfo->item.pszText, displayInfo->item.cchTextMax, value, _TRUNCATE);
}

struct AppSettingPagePlugins::PImpl
{
	PluginListDialog mWindow;
	std::unique_ptr<PluginSettings> mSettings;
};

REGISTER_APPSETTINGPAGE(AppSettingPagePlugins)

AppSettingPagePlugins::AppSettingPagePlugins() :
	AppSettingPageBase(_T(""), _T("プラグイン")), in(new PImpl)
{
}

AppSettingPagePlugins::~AppSettingPagePlugins()
{
}

bool AppSettingPagePlugins::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_PLUGINS, CWnd::FromHandle(parentWindow)) != FALSE;
}

HWND AppSettingPagePlugins::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

int AppSettingPagePlugins::GetOrder()
{
	return 95;
}

bool AppSettingPagePlugins::OnEnterSettings()
{
	in->mSettings = PluginSettings::GetInstance()->Clone();
	in->mWindow.SetSettings(in->mSettings.get());
	return true;
}

bool AppSettingPagePlugins::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

bool AppSettingPagePlugins::OnKillActive()
{
	return in->mWindow.OnKillActive();
}

void AppSettingPagePlugins::OnOKCall()
{
	in->mWindow.OnOK();
}

bool AppSettingPagePlugins::GetHelpPageId(String& id)
{
	id = "PluginSetting";
	return true;
}

} // namespace plugin
} // namespace commands
} // namespace launcherapp
