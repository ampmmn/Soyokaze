#include "pch.h"
#include "PlaceWindowInRegionCommandEditor.h"
#include "commands/place_window_in_region/PlaceWindowInRegionSettingDialog.h"

namespace launcherapp {
namespace commands {
namespace place_window_in_region {

using namespace launcherapp::core;

struct PlaceWindowInRegionCommandEditor::PImpl
{
	PImpl(CWnd* parentWnd) : mDialog(parentWnd)
	{
	}

	SettingDialog mDialog;
	uint32_t mRefCount{1};
	
};

PlaceWindowInRegionCommandEditor::PlaceWindowInRegionCommandEditor(CWnd* parentWnd) :
 	in(new PImpl(parentWnd))
{
}

PlaceWindowInRegionCommandEditor::~PlaceWindowInRegionCommandEditor()
{
}

void PlaceWindowInRegionCommandEditor::SetParam(const CommandParam& param)
{
	return in->mDialog.SetParam(param);
}

const CommandParam& PlaceWindowInRegionCommandEditor::GetParam()
{
	return in->mDialog.GetParam();
}

// 名前を上書きする
void PlaceWindowInRegionCommandEditor::OverrideName(LPCTSTR name) 
{
	in->mDialog.SetName(name);
	in->mDialog.ResetHotKey();
}

// 元のコマンド名を設定する(そのコマンド名と同じ場合は「コマンド名重複」とみなさない)
void PlaceWindowInRegionCommandEditor::SetOriginalName(LPCTSTR name) 
{
	in->mDialog.SetOriginalName(name);
}

// コマンドを編集するためのダイアログを作成/取得する
bool PlaceWindowInRegionCommandEditor::DoModal() 
{
	return in->mDialog.DoModal() == IDOK;
}


// UnknownIF
bool PlaceWindowInRegionCommandEditor::QueryInterface(const IFID& ifid, void** obj) 
{
	if (ifid == IFID_PLACEWINDOWINREGIONCOMMANDEDITOR) {
		*obj = (PlaceWindowInRegionCommandEditor*)this;
		AddRef();
		return true;
	}
	return false;
}

uint32_t PlaceWindowInRegionCommandEditor::AddRef() 
{
	return (uint32_t)InterlockedIncrement(&in->mRefCount);
}

uint32_t PlaceWindowInRegionCommandEditor::Release() 
{
	auto n = InterlockedDecrement(&in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
}

} // end of namespace place_window_in_region
} // end of namespace commands
} // end of namespace launcherapp

