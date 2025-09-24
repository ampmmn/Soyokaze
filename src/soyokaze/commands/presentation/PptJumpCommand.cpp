#include "pch.h"
#include "framework.h"
#include "PptJumpCommand.h"
#include "commands/presentation/PowerPointWindow.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/builtin/CallbackAction.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;

namespace launcherapp {
namespace commands {
namespace presentation {

struct PptJumpCommand::PImpl
{
	int mPage{0};
	CString mTitle;
	CString mFilePath;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(PptJumpCommand)

PptJumpCommand::PptJumpCommand(
		const CString& filePath,
		int page,
	 	const CString& title
) : in(std::make_unique<PImpl>())
{
	in->mPage = page;
	in->mTitle = title;
	in->mFilePath = filePath;

	this->mName.Format(_T("%s (%s)"), (LPCTSTR)title, PathFindFileName(filePath));
	this->mDescription.Format(_T("スライド%d: %s"), page, (LPCTSTR)title);
}

PptJumpCommand::~PptJumpCommand()
{
}


CString PptJumpCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool PptJumpCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0 || modifierFlags == Command::MODIFIER_CTRL) {

		auto slideNo = (uint16_t)in->mPage;
		bool isShowMaximize = (modifierFlags == Command::MODIFIER_CTRL);

		LPCTSTR guideText = isShowMaximize == false ? _T("スライドを表示する") : _T("最大化表示");

		*action = new CallbackAction(guideText, [slideNo, isShowMaximize](Parameter*, String*) -> bool {

			// 現在表示中のパワポを取得
			std::unique_ptr<PowerPointWindow> pptWnd;
			PowerPointWindow::GetAcitveWindow(pptWnd);
	
			// スライドにジャンプ
			pptWnd->GoToSlide(slideNo);
	
			// 表示しているパワポのウインドウをアクティブにする
			pptWnd->Activate(isShowMaximize);
			return true;
		});
		return true;
	}
	return false;
}

HICON PptJumpCommand::GetIcon()
{
	// 拡張子に関連付けられたアイコンを取得
	LPCTSTR fileExt = PathFindExtension(in->mFilePath);
	if (_tcslen(fileExt) == 0) {
		return IconLoader::Get()->LoadUnknownIcon();
	}
	return IconLoader::Get()->LoadExtensionIcon(fileExt);
}

launcherapp::core::Command*
PptJumpCommand::Clone()
{
	return new PptJumpCommand(in->mFilePath, in->mPage, in->mTitle);
}

CString PptJumpCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)_T("スライド検索"));
	return TEXT_TYPE;
}

} // end of namespace presentation
} // end of namespace commands
} // end of namespace launcherapp

