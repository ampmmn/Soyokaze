#include "pch.h"
#include "framework.h"
#include "PyExtensionCommand.h"
#include "core/IFIDDefine.h"
#include "commands/py_extension/PyExtensionCommandEditor.h"
#include "commands/py_extension/PyEvalAction.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "hotkey/CommandHotKeyManager.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"
#include "python/PythonDLLLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp { namespace commands { namespace py_extension {

using CommandRepository = launcherapp::core::CommandRepository;

struct PyExtensionCommand::PImpl
{
	PImpl()
	{
	}
	~PImpl()
	{
	}

	CommandParam mParam;
	// 実行可能か?
	bool mCanExecute{false};
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString PyExtensionCommand::GetType() { return _T("PyExtension"); }

CString PyExtensionCommand::TypeDisplayName()
{
	static CString TEXT_TYPE(_T("Python拡張コマンド"));
	return TEXT_TYPE;
}

PyExtensionCommand::PyExtensionCommand() : in(std::make_unique<PImpl>())
{
}

PyExtensionCommand::~PyExtensionCommand()
{
}

CString PyExtensionCommand::GetName()
{
	return in->mParam.mName;
}


CString PyExtensionCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString PyExtensionCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}


// 修飾キー押下状態に対応した実行アクションを取得する
bool PyExtensionCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	UNREFERENCED_PARAMETER(action);

	if (hotkeyAttr.GetModifiers() != 0) {
		return false;
	}

	*action = new PyEvalAction(&in->mParam);
	return true;
}


void PyExtensionCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;
}

HICON PyExtensionCommand::GetIcon()
{
	auto loader = PythonDLLLoader::Get();
	return IconLoader::Get()->LoadIconFromPath(loader->GetPythonExePath());
}

int PyExtensionCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool PyExtensionCommand::IsAllowAutoExecute()
{
	return false;
}

bool PyExtensionCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
PyExtensionCommand::Clone()
{
	auto clonedObj = make_refptr<PyExtensionCommand>();
	clonedObj->in->mParam = in->mParam;
	return clonedObj.release();
}

bool PyExtensionCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	return in->mParam.Save(entry);
}

bool PyExtensionCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != GetType()) {
		return false;
	}
	return in->mParam.Load(entry);
}

bool PyExtensionCommand::NewInstance(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_PYEXTENSIONCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<PyExtensionCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

bool PyExtensionCommand::NewDialog(Parameter* param, PyExtensionCommand** newCmdPtr)
{
	// 新規作成ダイアログを表示
	CString value;
	CommandParam paramTmp;

	if (GetNamedParamString(param, _T("COMMAND"), value)) {
		paramTmp.mName = value;
	}
	if (GetNamedParamString(param, _T("DESCRIPTION"), value)) {
		paramTmp.mDescription = value;
	}
	if (GetNamedParamString(param, _T("TEXT"), value)) {
		paramTmp.mScript = value;
	}

	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	cmdEditor->SetParam(paramTmp);
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<PyExtensionCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;

}

// コマンドを編集するためのダイアログを作成/取得する
bool PyExtensionCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
{
	if (editor == nullptr) {
		return false;
	}

	auto cmdEditor = new CommandEditor(CWnd::FromHandle(parent));
	cmdEditor->SetParam(in->mParam);

	*editor = cmdEditor;
	return true;
}

bool PyExtensionCommand::LoadFrom(CommandFile* cmdFile, void* e, PyExtensionCommand** newCmdPtr)
{
	UNREFERENCED_PARAMETER(cmdFile);

	ASSERT(newCmdPtr);

	CommandFile::Entry* entry = (CommandFile::Entry*)e;

	auto command = make_refptr<PyExtensionCommand>();
	if (command->Load(entry) == false) {
		return false;
	}

	if (newCmdPtr) {
		*newCmdPtr = command.release();
	}
	return true;
}

// ダイアログ上での編集結果をコマンドに適用する
bool PyExtensionCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_PYEXTENSIONCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	SetParam(cmdEditor->GetParam());

	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool PyExtensionCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_PYEXTENSIONCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}
	auto newCmd = make_refptr<PyExtensionCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

}}}

