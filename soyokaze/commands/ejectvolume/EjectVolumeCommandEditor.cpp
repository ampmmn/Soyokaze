#include "pch.h"
#include "EjectVolumeCommandEditor.h"
#include "commands/ejectvolume/EjectVolumeEditDialog.h"

namespace launcherapp {
namespace commands {
namespace ejectvolume {

using namespace launcherapp::core;

struct EjectVolumeCommandEditor::PImpl
{
	PImpl(CWnd* parentWnd) : mDialog(parentWnd)
	{
	}

	SettingDialog mDialog;
	uint32_t mRefCount = 1;
	
};

EjectVolumeCommandEditor::EjectVolumeCommandEditor(CWnd* parentWnd) :
 	in(new PImpl(parentWnd))
{
}

EjectVolumeCommandEditor::~EjectVolumeCommandEditor()
{
}

void EjectVolumeCommandEditor::SetParam(const CommandParam& param)
{
	return in->mDialog.SetParam(param);
}

const CommandParam& EjectVolumeCommandEditor::GetParam()
{
	return in->mDialog.GetParam();
}

// コマンドは編集可能か?
void EjectVolumeCommandEditor::SetOriginalName(LPCTSTR name) 
{
	in->mDialog.SetOriginalName(name);
}

// コマンドを編集するためのダイアログを作成/取得する
bool EjectVolumeCommandEditor::DoModal() 
{
	return in->mDialog.DoModal() != IDOK;
}


// UnknownIF
bool EjectVolumeCommandEditor::QueryInterface(const IFID& ifid, void** obj) 
{
	if (ifid == IFID_EJECTVOLUMECOMMANDEDITOR) {
		*obj = (EjectVolumeCommandEditor*)this;
		AddRef();
		return true;
	}
	return false;
}

uint32_t EjectVolumeCommandEditor::AddRef() 
{
	return (uint32_t)InterlockedIncrement(&in->mRefCount);
}

uint32_t EjectVolumeCommandEditor::Release() 
{
	auto n = InterlockedDecrement(&in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
}

} // end of namespace ejectvolume
} // end of namespace commands
} // end of namespace launcherapp

