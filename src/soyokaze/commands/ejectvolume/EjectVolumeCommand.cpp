#include "pch.h"
#include "framework.h"
#include "EjectVolumeCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/ejectvolume/EjectVolumeCommandParam.h"
#include "commands/ejectvolume/EjectVolumeCommandEditor.h"
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

constexpr LPCTSTR TYPENAME = _T("EjectVolumeCommand");

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

BOOL EjectVolumeCommand::Execute(Parameter* param_)
{
	UNUSED(param_);
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

bool EjectVolumeCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
EjectVolumeCommand::Clone()
{
	auto clonedObj = make_refptr<EjectVolumeCommand>();

	clonedObj->in->mParam = in->mParam;

	return clonedObj.release();
}

bool EjectVolumeCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	entry->Set(_T("description"), GetDescription());
	entry->Set(_T("DriveLetter"), (int)in->mParam.mDriveLetter);

	return true;
}

bool EjectVolumeCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != EjectVolumeCommand::GetType()) {
		return false;
	}

	in->mParam.mName = entry->GetName();
	in->mParam.mDescription = entry->Get(_T("description"), _T(""));

	TCHAR letter = (TCHAR)entry->Get(_T("DriveLetter"), 0);
	if (letter < _T('A') || _T('Z') < letter) {
		return false;
	}
	in->mParam.mDriveLetter = letter;

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mParam.mHotKeyAttr); 

	return true;
}

bool EjectVolumeCommand::NewDialog(Parameter* param)
{
	param;  // 非サポート

	RefPtr<EjectVolumeCommandEditor> cmdEditor(new EjectVolumeCommandEditor());
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	auto& paramNew = cmdEditor->GetParam();

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<EjectVolumeCommand>();
	newCmd->SetParam(paramNew);

	constexpr bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd.release(), isReloadHotKey);

	return true;

}

void EjectVolumeCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;
}

// コマンドを編集するためのダイアログを作成/取得する
bool EjectVolumeCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
{
	if (editor == nullptr) {
		return false;
	}

	auto cmdEditor = new EjectVolumeCommandEditor(CWnd::FromHandle(parent));
	cmdEditor->SetParam(in->mParam);

	*editor = cmdEditor;
	return true;
}

// ダイアログ上での編集結果をコマンドに適用する
bool EjectVolumeCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<EjectVolumeCommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_EJECTVOLUMECOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	in->mParam = cmdEditor->GetParam();
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool EjectVolumeCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<EjectVolumeCommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_EJECTVOLUMECOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	auto paramNew = cmdEditor->GetParam();

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<EjectVolumeCommand>();
	newCmd->SetParam(paramNew);

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

}
}
}

