#include "pch.h"
#include "framework.h"
#include "SettingCommand.h"
#include "SettingDialog.h"
#include "AppPreference.h"
#include "HotKeyAttribute.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SettingCommand::SettingCommand()
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
	dlg.mMatchLevel = pref->mMatchLevel;
	dlg.mIsUseExternalFiler = pref->mIsUseExternalFiler;
	dlg.mFilerPath = pref->mFilerPath;
	dlg.mFilerParam = pref->mFilerParam;
	dlg.mIsTopMost = pref->mIsTopmost;
	dlg.mHotKeyAttr = HOTKEY_ATTR(pref->mModifiers, pref->mHotKeyVK);
	dlg.mAlpha = pref->mAlpha;

	if (pref->mIsTransparencyEnable == false) {
		dlg.mTransparencyType = 2;
	}
	else if (pref->mIsTransparencyInactiveOnly) {
		dlg.mTransparencyType = 0;
	}
	else {
		dlg.mTransparencyType = 1;
	}

	if (dlg.DoModal() != IDOK) {
		return TRUE;
	}

	// 設定変更を反映する
	pref->mMatchLevel = dlg.mMatchLevel;
	pref->mIsUseExternalFiler = dlg.mIsUseExternalFiler;
	pref->mFilerPath = dlg.mFilerPath;
	pref->mFilerParam = dlg.mFilerParam;
	pref->mIsTopmost = dlg.mIsTopMost;
	pref->mModifiers = dlg.mHotKeyAttr.GetModifiers();
	pref->mHotKeyVK = dlg.mHotKeyAttr.GetVKCode();
	pref->mAlpha = dlg.mAlpha;

	if (dlg.mTransparencyType == 0) {
		pref->mIsTransparencyEnable = true;
		pref->mIsTransparencyInactiveOnly = true;
	}
	else if (dlg.mTransparencyType == 1) {
		pref->mIsTransparencyEnable = true;
		pref->mIsTransparencyInactiveOnly = false;
	}
	else {
		pref->mIsTransparencyEnable = false;
		pref->mIsTransparencyInactiveOnly = true;
	}

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


BOOL SettingCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

Command* SettingCommand::Clone()
{
	return new SettingCommand();
}

