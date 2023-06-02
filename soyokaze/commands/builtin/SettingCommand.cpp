#include "pch.h"
#include "framework.h"
#include "SettingCommand.h"
#include "gui/SettingDialog.h"
#include "AppPreference.h"
#include "HotKeyAttribute.h"
#include "CommandFile.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

CString SettingCommand::GetType() { return _T("Builtin-Setting"); }

SettingCommand::SettingCommand(LPCTSTR name) :
	mRefCount(1)
{
	mName = name ? name : _T("setting");
}

SettingCommand::~SettingCommand()
{
}

CString SettingCommand::GetName()
{
	return mName;
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

BOOL SettingCommand::Execute(const Parameter& param)
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

bool SettingCommand::IsEditable()
{
	return false;
}


int SettingCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command* SettingCommand::Clone()
{
	return new SettingCommand();
}

bool SettingCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);
	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	return true;
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

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

