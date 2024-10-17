#include "pch.h"
#include "BookmarkCommandEditor.h"
#include "commands/bookmark/BookmarkSettingDialog.h"

namespace launcherapp {
namespace commands {
namespace bookmarks {

using namespace launcherapp::core;

struct BookmarkCommandEditor::PImpl
{
	PImpl(CWnd* parentWnd) : mDialog(parentWnd)
	{
	}

	SettingDialog mDialog;
	uint32_t mRefCount = 1;
	
};

BookmarkCommandEditor::BookmarkCommandEditor(CWnd* parentWnd) :
 	in(new PImpl(parentWnd))
{
}

BookmarkCommandEditor::~BookmarkCommandEditor()
{
}

void BookmarkCommandEditor::SetParam(const CommandParam& param)
{
	return in->mDialog.SetParam(param);
}

const CommandParam& BookmarkCommandEditor::GetParam()
{
	return in->mDialog.GetParam();
}

// コマンドは編集可能か?
void BookmarkCommandEditor::SetOriginalName(LPCTSTR name) 
{
	in->mDialog.SetOriginalName(name);
}

// コマンドを編集するためのダイアログを作成/取得する
bool BookmarkCommandEditor::DoModal() 
{
	return in->mDialog.DoModal() != IDOK;
}


// UnknownIF
bool BookmarkCommandEditor::QueryInterface(const IFID& ifid, void** obj) 
{
	if (ifid == IFID_BOOKMARKCOMMANDEDITOR) {
		*obj = (BookmarkCommandEditor*)this;
		AddRef();
		return true;
	}
	return false;
}

uint32_t BookmarkCommandEditor::AddRef() 
{
	return (uint32_t)InterlockedIncrement(&in->mRefCount);
}

uint32_t BookmarkCommandEditor::Release() 
{
	auto n = InterlockedDecrement(&in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
}

} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

