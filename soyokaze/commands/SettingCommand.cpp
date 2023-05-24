#include "pch.h"
#include "framework.h"
#include "SettingCommand.h"
#include "gui/SettingDialog.h"
#include "AppPreference.h"
#include "HotKeyAttribute.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SettingCommand::SettingCommand() :
	mRefCount(1)
{
}

SettingCommand::~SettingCommand()
{
}

CString SettingCommand::GetName()
{
	return _T("setting");
}

CString SettingCommand::GetDescription()
{
	return _T("【設定】");
}

BOOL SettingCommand::Execute()
{
	SettingDialog dlg;

	auto pref = AppPreference::Get();

	// 現在の設定をセット
	dlg.SetSettings(pref->GetSettings());

	if (dlg.DoModal() != IDOK) {
		return TRUE;
	}

	// 設定変更を反映する
	pref->SetSettings(dlg.GetSettings());
	pref->Save();

	return TRUE;
}

BOOL SettingCommand::Execute(const std::vector<CString>& args)
{
	// 引数指定しても動作はかわらない
	return Execute();
}

CString SettingCommand::GetErrorString()
{
	return _T("");
}

HICON SettingCommand::GetIcon()
{
	return IconLoader::Get()->LoadSettingIcon();

}


int SettingCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

soyokaze::core::Command* SettingCommand::Clone()
{
	return new SettingCommand();
}

uint32_t SettingCommand::AddRef()
{
	return ++mRefCount;
}

uint32_t SettingCommand::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}
