#include "pch.h"
#include "framework.h"
#include "SettingDialog.h"
#include "commands/core/CommandRepository.h"
#include "settingwindow/BasicSettingDialog.h"
#include "settingwindow/InputSettingDialog.h"
#include "settingwindow/InputHistorySettingPage.h"
#include "settingwindow/InputWindowKeySettingPage.h"
#include "settingwindow/ExecSettingDialog.h"
#include "settingwindow/ExcludePathPage.h"
#include "settingwindow/AppSettingPathPage.h"
#include "settingwindow/ViewSettingDialog.h"
#include "settingwindow/SoundSettingDialog.h"
#include "settingwindow/ExtensionSettingDialog.h"
#include "settingwindow/ShortcutSettingPage.h"
#include "settingwindow/AppSettingPageOther.h"
#include "settingwindow/AppSettingPageCommandPriority.h"
#include "utility/TopMostMask.h"
#include "resource.h"
#include <algorithm>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;

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

	auto hInputItem = AddPage(TVI_ROOT, std::unique_ptr<SettingPage>(new InputSettingDialog(this)), param);
	AddPage(hInputItem, std::unique_ptr<SettingPage>(new InputWindowKeySettingPage(this)), param);
	AddPage(hInputItem, std::unique_ptr<SettingPage>(new InputHistorySettingPage(this)), param);
	auto hExecItem = AddPage(TVI_ROOT, std::unique_ptr<SettingPage>(new ExecSettingDialog(this)), param);
	AddPage(hExecItem, std::unique_ptr<SettingPage>(new AppSettingPathPage(this)), param);
	AddPage(hExecItem, std::unique_ptr<SettingPage>(new ExcludePathPage(this)), param);
	AddPage(hExecItem, std::unique_ptr<SettingPage>(new AppSettingPageCommandPriority(this)), param);

	AddPage(TVI_ROOT, std::unique_ptr<SettingPage>(new ViewSettingDialog(this)), param);

	auto hExtensionItem = AddPage(TVI_ROOT, std::unique_ptr<SettingPage>(new ExtensionSettingDialog(this)), param);

	std::vector<SettingPage*> pages;
	CommandRepository::GetInstance()->EnumProviderSettingDialogs(this, pages);
		// EnumProviderSettingDialogsで取得した各ページの解放は呼び出し側で行う
	for (auto page : pages) {
		AddPage(hExtensionItem, std::unique_ptr<SettingPage>(page), param);
	}

	AddPage(TVI_ROOT, std::unique_ptr<SettingPage>(new AppSettingPageOther(this)), param);
	return hItem;
}


