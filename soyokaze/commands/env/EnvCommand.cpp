#include "pch.h"
#include "framework.h"
#include "EnvCommand.h"
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
	CString mName;
	CString mValue;
	uint32_t mRefCount;
};


EnvCommand::EnvCommand(const CString& name, const CString& value) : in(new PImpl)
{
	in->mName = name;
	in->mValue = value;
	in->mRefCount = 1;
}

EnvCommand::~EnvCommand()
{
}

CString EnvCommand::GetName()
{
	return in->mName;
}

CString EnvCommand::GetDescription()
{
	return in->mValue;
}

CString EnvCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_ENVIRON);
	return TEXT_TYPE;
}

BOOL EnvCommand::Execute()
{
	Parameter emptyParams;
	return Execute(emptyParams);
}

BOOL EnvCommand::Execute(const Parameter& param)
{
	// クリップボードにコピー
	size_t bufLen = sizeof(TCHAR) * (in->mValue.GetLength() + 1);
	HGLOBAL hMem = GlobalAlloc(GHND | GMEM_SHARE , bufLen);
	LPTSTR p = (LPTSTR)GlobalLock(hMem);
	_tcscpy_s(p, bufLen, in->mValue);
	GlobalUnlock(hMem);

	BOOL isSet=FALSE;
	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 9, (WPARAM)&isSet, (LPARAM)hMem);

	if (isSet == FALSE) {
		GlobalFree(hMem);
	}

	return TRUE;
}

CString EnvCommand::GetErrorString()
{
	return _T("");
}

HICON EnvCommand::GetIcon()
{
	return IconLoader::Get()->LoadDefaultIcon();
}

int EnvCommand::Match(Pattern* pattern)
{
	// サポートしない
	return Pattern::WholeMatch;
}

bool EnvCommand::IsEditable()
{
	return false;
}

int EnvCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command*
EnvCommand::Clone()
{
	auto clonedObj = new EnvCommand(in->mName, in->mValue);
	return clonedObj;
}

bool EnvCommand::Save(CommandFile* cmdFile)
{
	// 非サポート
	return false;
}

uint32_t EnvCommand::AddRef()
{
	return ++(in->mRefCount);
}

uint32_t EnvCommand::Release()
{
	auto n = --(in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace env
} // end of namespace commands
} // end of namespace soyokaze

