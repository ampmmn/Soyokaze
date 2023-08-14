#include "pch.h"
#include "framework.h"
#include "EnvCommand.h"
#include "utility/GlobalAllocMemory.h"
#include "IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace env {


struct EnvCommand::PImpl
{
	CString mValue;
};


EnvCommand::EnvCommand(const CString& name, const CString& value) : 
	AdhocCommandBase(name, value),
	in(new PImpl)
{
	in->mValue = value;
}

EnvCommand::~EnvCommand()
{
}

CString EnvCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_ENVIRON);
	return TEXT_TYPE;
}

BOOL EnvCommand::Execute(const Parameter& param)
{
	// クリップボードにコピー
	size_t bufLen = sizeof(TCHAR) * (in->mValue.GetLength() + 1);
	GlobalAllocMemory mem(bufLen);
	_tcscpy_s((LPTSTR)mem.Lock(), bufLen, in->mValue);
	mem.Unlock();

	BOOL isSet=FALSE;
	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 9, (WPARAM)&isSet, (LPARAM)(HGLOBAL)mem);

	if (isSet) {
		mem.Release();
	}

	return TRUE;
}

HICON EnvCommand::GetIcon()
{
	return IconLoader::Get()->LoadDefaultIcon();
}

soyokaze::core::Command*
EnvCommand::Clone()
{
	auto clonedObj = new EnvCommand(this->mName, in->mValue);
	return clonedObj;
}

} // end of namespace env
} // end of namespace commands
} // end of namespace soyokaze

