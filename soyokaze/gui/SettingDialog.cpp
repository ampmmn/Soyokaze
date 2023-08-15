#include "pch.h"
#include "framework.h"
#include "SettingDialog.h"
#include "gui/ShortcutDialog.h"
#include "gui/BasicSettingDialog.h"
#include "gui/InputSettingDialog.h"
#include "gui/ExecSettingDialog.h"
#include "gui/ViewSettingDialog.h"
#include "gui/SoundSettingDialog.h"
#include "gui/ExtensionSettingDialog.h"
#include "gui/ShortcutSettingPage.h"
#include "utility/TopMostMask.h"
#include "resource.h"
#include <algorithm>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct SettingDialog::PImpl
{
	// 設定情報
	Settings mSettings;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



SettingDialog::SettingDialog() : in(std::make_unique<PImpl>())
{
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::SetSettings(const Settings& settings)
{
	std::unique_ptr<Settings> tmp(settings.Clone());
	in->mSettings.Swap(*tmp.get());
}

const Settings& SettingDialog::GetSettings()
{
	return in->mSettings;
}


void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(SettingDialog, SettingDialogBase)
END_MESSAGE_MAP()


BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();
	return TRUE;
}

void SettingDialog::OnOK()
{
	__super::OnOK();
}

HTREEITEM SettingDialog::OnSetupPages()
{

	void* param = &(in->mSettings);

	// 各ページ作成
	HTREEITEM hItem = AddPage(TVI_ROOT, std::unique_ptr<SettingPage>(new BasicSettingDialog(this)), param);
	AddPage(hItem, std::unique_ptr<SettingPage>(new ShortcutSettingPage(this)), param);
	AddPage(hItem, std::unique_ptr<SettingPage>(new SoundSettingDialog(this)), param);

	AddPage(TVI_ROOT, std::unique_ptr<SettingPage>(new InputSettingDialog(this)), param);
	AddPage(TVI_ROOT, std::unique_ptr<SettingPage>(new ExecSettingDialog(this)), param);
	AddPage(TVI_ROOT, std::unique_ptr<SettingPage>(new ViewSettingDialog(this)), param);

	AddPage(TVI_ROOT, std::unique_ptr<SettingPage>(new ExtensionSettingDialog(this)), param);

	return hItem;
}


