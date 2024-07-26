#include "pch.h"
#include "framework.h"
#include "PptJumpCommand.h"
#include "commands/presentation/PowerPointWindow.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace presentation {

struct PptJumpCommand::PImpl
{
	int mPage = 0;
	CString mTitle;
};


PptJumpCommand::PptJumpCommand(
		int page,
	 	const CString& title
) : in(std::make_unique<PImpl>())
{
	in->mPage = page;
	in->mTitle = title;

	this->mName = title;
	this->mDescription.Format(_T("スライド%d: %s"), page, (LPCTSTR)title);
}

PptJumpCommand::~PptJumpCommand()
{
}

CString PptJumpCommand::GetGuideString()
{
	return _T("Enter:スライドを表示する Ctrl-Enter:最大化表示");
}


CString PptJumpCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)_T("スライド検索"));
	return TEXT_TYPE;
}

BOOL PptJumpCommand::Execute(const Parameter& param)
{
	// 現在表示中のパワポを取得
	std::unique_ptr<PowerPointWindow> pptWnd;
	PowerPointWindow::GetAcitveWindow(pptWnd);

	// スライドにジャンプ
	pptWnd->GoToSlide((int16_t)in->mPage);

	// 表示しているパワポのウインドウをアクティブにする
	bool isShowMaximize = param.GetNamedParamBool(_T("CtrlKeyPressed"));
	pptWnd->Activate(isShowMaximize);

	return TRUE;
}

HICON PptJumpCommand::GetIcon()
{
	// ToDo: 実装
	return PowerPointWindow::ResolveIcon();
}

launcherapp::core::Command*
PptJumpCommand::Clone()
{
	return new PptJumpCommand(in->mPage, in->mTitle);
}

} // end of namespace presentation
} // end of namespace commands
} // end of namespace launcherapp

