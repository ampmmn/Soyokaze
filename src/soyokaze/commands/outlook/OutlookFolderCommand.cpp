#include "pch.h"
#include "framework.h"
#include "OutlookFolderCommand.h"
#include "commands/outlook/OutlookProxy.h"
#include "actions/builtin/CallbackAction.h"
#include "actions/core/ActionParameter.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using namespace launcherapp::actions::builtin;
using namespace launcherapp::actions::core;

namespace launcherapp { namespace commands { namespace outlook {

struct OutlookFolderCommand::PImpl
{
	CComPtr<IDispatch> mFolder;
	int mCount{0};
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(OutlookFolderCommand)


OutlookFolderCommand::OutlookFolderCommand(
	const CString& fullName,
 	int itemCount,
 	CComPtr<IDispatch>& folder
) : in(std::make_unique<PImpl>())
{
	mName = fullName;
	mDescription.Format(_T("%s(%d)"), (LPCTSTR)fullName, itemCount);

	in->mCount = itemCount;
	in->mFolder = folder;

}

OutlookFolderCommand::~OutlookFolderCommand()
{
}

CString OutlookFolderCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool OutlookFolderCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags != 0) {
		return false;
	}

	*action = new CallbackAction(_T("フォルダを開く"), [&](Parameter*, String*) -> bool {
			return OutlookProxy::GetInstance()->SelectFolder(in->mFolder);
	});

	return true;
}

HICON OutlookFolderCommand::GetIcon()
{
	return IconLoader::Get()->LoadFolderIcon();
}

launcherapp::core::Command*
OutlookFolderCommand::Clone()
{
	return new OutlookFolderCommand(mName, in->mCount, in->mFolder);
}

CString OutlookFolderCommand::TypeDisplayName()
{
	return _T("Outlookフォルダ");
}

}}} // end of namespace launcherapp::commands::outlook
