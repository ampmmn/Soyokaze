#include "pch.h"
#include "framework.h"
#include "EjectVolumeCommand.h"
#include "commands/ejectvolume/EjectVolumeCommandParam.h"
#include "commands/ejectvolume/EjectVolumeEditDialog.h"
#include "commands/ejectvolume/EjectVolume.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace ejectvolume {

using CommandRepository = launcherapp::core::CommandRepository;

struct EjectVolumeCommand::PImpl
{
	PImpl()
	{
	}
	~PImpl()
	{
	}

	CommandParam mParam;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString EjectVolumeCommand::GetType() { return _T("EjectVolume"); }

EjectVolumeCommand::EjectVolumeCommand() : in(std::make_unique<PImpl>())
{
}

EjectVolumeCommand::~EjectVolumeCommand()
{
}

CString EjectVolumeCommand::GetName()
{
	return in->mParam.mName;
}


CString EjectVolumeCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString EjectVolumeCommand::GetGuideString()
{
	CString guideStr;
	guideStr.Format(_T("Enter:%c:ドライブを取り外す"), in->mParam.mDriveLetter);
	return guideStr; 
}

CString EjectVolumeCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE(_T("取リ外しコマンド"));
	return TEXT_TYPE;
}

BOOL EjectVolumeCommand::Execute(const Parameter& param_)
{
	const auto& param = in->mParam;
	if (EjectVolume(param.mDriveLetter, nullptr, nullptr) == false) {
		spdlog::warn(_T("Failed to eject volume. {0} {1}:"), (LPCTSTR)param.mName, param.mDriveLetter);
		return TRUE;
	}

	return TRUE;
}

CString EjectVolumeCommand::GetErrorString()
{
	return _T("");
}

HICON EjectVolumeCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-5328);
}

int EjectVolumeCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

int EjectVolumeCommand::EditDialog(const Parameter* param)
{
	SettingDialog dlg;
	dlg.SetParam(in->mParam);

	if (dlg.DoModal() != IDOK) {
		return 1;
	}

	// ホットキー登録をいったん削除
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();

	CommandHotKeyMappings hotKeyMap;
	hotKeyManager->GetMappings(hotKeyMap);

	if (in->mParam.mHotKeyAttr.IsValid()) {
		hotKeyMap.RemoveItem(in->mParam.mHotKeyAttr);
	}

	// 更新後の設定値を取得
	in->mParam = dlg.GetParam();

	// 再登録
	auto cmdRepo = CommandRepository::GetInstance();
	cmdRepo->ReregisterCommand(this);

	// ホットキー設定を更新
	if (in->mParam.mHotKeyAttr.IsValid()) {
		hotKeyMap.AddItem(in->mParam.mName, in->mParam.mHotKeyAttr, in->mParam.mIsGlobal);
	}
	auto pref = AppPreference::Get();
	pref->SetCommandKeyMappings(hotKeyMap);
	pref->Save();

	return 0;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool EjectVolumeCommand::IsPriorityRankEnabled()
{
	return true;
}

launcherapp::core::Command*
EjectVolumeCommand::Clone()
{
	auto clonedObj = std::make_unique<EjectVolumeCommand>();

	clonedObj->in->mParam = in->mParam;

	return clonedObj.release();
}

bool EjectVolumeCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());

	cmdFile->Set(entry, _T("description"), GetDescription());
	cmdFile->Set(entry, _T("DriveLetter"), (int)in->mParam.mDriveLetter);

	return true;
}

bool EjectVolumeCommand::Load(CommandFile* cmdFile, void* entry_)
{
	auto entry = (CommandFile::Entry*)entry_;

	in->mParam.mName = cmdFile->GetName(entry);
	in->mParam.mDescription = cmdFile->Get(entry, _T("description"), _T(""));

	TCHAR letter = (TCHAR)cmdFile->Get(entry, _T("DriveLetter"), 0);
	if (letter < _T('A') || _T('Z') < letter) {
		return false;
	}
	in->mParam.mDriveLetter = letter;

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mParam.mHotKeyAttr,&in->mParam.mIsGlobal); 

	return true;
}

bool EjectVolumeCommand::NewDialog(const Parameter* param)
{
	param;  // 非サポート

	SettingDialog dlg;
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	auto& paramNew = dlg.GetParam();

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = std::make_unique<EjectVolumeCommand>();
	newCmd->SetParam(paramNew);

	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());

	// ホットキー設定を更新
	if (paramNew.mHotKeyAttr.IsValid()) {

		auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
		CommandHotKeyMappings hotKeyMap;
		hotKeyManager->GetMappings(hotKeyMap);

		hotKeyMap.AddItem(paramNew.mName, paramNew.mHotKeyAttr, paramNew.mIsGlobal);

		auto pref = AppPreference::Get();
		pref->SetCommandKeyMappings(hotKeyMap);

		pref->Save();
	}

	return true;

}

void EjectVolumeCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;
}

}
}
}

