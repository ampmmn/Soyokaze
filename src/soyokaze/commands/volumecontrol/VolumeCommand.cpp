#include "pch.h"
#include "framework.h"
#include "VolumeCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/volumecontrol/VolumeCommandParam.h"
#include "commands/volumecontrol/VolumeCommandEditor.h"
#include "commands/volumecontrol/AudioSessionVolume.h"
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
namespace volumecontrol {

using CommandRepository = launcherapp::core::CommandRepository;

struct VolumeCommand::PImpl
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

CString VolumeCommand::GetType() { return _T("Volume"); }

VolumeCommand::VolumeCommand() : in(std::make_unique<PImpl>())
{
}

VolumeCommand::~VolumeCommand()
{
}

CString VolumeCommand::GetName()
{
	return in->mParam.mName;
}


CString VolumeCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString VolumeCommand::GetGuideString()
{
	return _T("⏎:音量設定を変更");
}

CString VolumeCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE(_T("音量設定"));
	return TEXT_TYPE;
}

BOOL VolumeCommand::Execute(Parameter* param_)
{
	UNREFERENCED_PARAMETER(param_);

	AudioSessionVolume vol;

	const auto& param = in->mParam;

	if (param.mIsSetVolume) {

		if (param.mIsRelative == FALSE) {
			vol.SetVolume(param.GetVolumeLevel());
		}
		else {
			int level = 0;
			vol.GetVolume(&level);
			level += param.GetVolumeLevel();
			vol.SetVolume(level);
		}
	}

	if (param.mMuteControl == 1) { // ミュート解除
		vol.SetMute(FALSE);
	}
	else if (param.mMuteControl == 2) { // ミュート
		vol.SetMute(TRUE);
	}
	else if (param.mMuteControl == 3) { // トグル
		BOOL isMute = FALSE;
		vol.GetMute(&isMute);
		vol.SetMute(!isMute);
	}

	// ToDo: 実装
	return TRUE;
}

CString VolumeCommand::GetErrorString()
{
	return _T("");
}

HICON VolumeCommand::GetIcon()
{
	return IconLoader::Get()->LoadVolumeIcon(false);
}

int VolumeCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool VolumeCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
VolumeCommand::Clone()
{
	auto clonedObj = make_refptr<VolumeCommand>();

	clonedObj->in->mParam = in->mParam;

	return clonedObj.release();
}

bool VolumeCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());

	entry->Set(_T("description"), GetDescription());
	entry->Set(_T("IsSetVolume"), in->mParam.mIsSetVolume? true : false);
	entry->Set(_T("VolumeLevel"), in->mParam.mVolume);
	entry->Set(_T("IsVolumeRelative"), in->mParam.mIsRelative);
	entry->Set(_T("MuteControlType"), in->mParam.mMuteControl);

	return true;
}

bool VolumeCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr !=GetType()) {
		return false;
	}

	in->mParam.mName = entry->GetName();
	in->mParam.mDescription = entry->Get(_T("description"), _T(""));

	in->mParam.mIsSetVolume = entry->Get(_T("IsSetVolume"), false) ? TRUE : FALSE;
	in->mParam.mVolume = entry->Get(_T("VolumeLevel") , 0); 
	in->mParam.mIsRelative = entry->Get(_T("VolumeLevel") , false); 
	in->mParam.mMuteControl = entry->Get(_T("MuteControlType") , 0); 

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mParam.mHotKeyAttr); 

	return true;
}

bool VolumeCommand::NewDialog(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<VolumeCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());

	return true;

}

void VolumeCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;
}

// コマンドを編集するためのダイアログを作成/取得する
bool VolumeCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
{
	if (editor == nullptr) {
		return false;
	}

	auto cmdEditor = new CommandEditor(CWnd::FromHandle(parent));
	cmdEditor->SetParam(in->mParam);

	*editor = cmdEditor;
	return true;
}

// ダイアログ上での編集結果をコマンドに適用する
bool VolumeCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_VOLUMECOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	in->mParam = cmdEditor->GetParam();
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool VolumeCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_VOLUMECOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	auto paramNew = cmdEditor->GetParam();

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<VolumeCommand>();
	newCmd->SetParam(paramNew);

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}


}
}
}

