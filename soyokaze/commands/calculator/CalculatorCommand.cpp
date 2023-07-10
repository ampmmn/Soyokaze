#include "pch.h"
#include "framework.h"
#include "CalculatorCommand.h"
#include "commands/common/ExecuteHistory.h"
#include "SharedHwnd.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace calculator {


struct CalculatorCommand::PImpl
{
	CString mResult;
	CString mDescription;

	TCHAR mCalcPath[MAX_PATH_NTFS];

	uint32_t mRefCount;
};


CalculatorCommand::CalculatorCommand() : in(new PImpl)
{
	in->mRefCount = 1;
	in->mCalcPath[0] = _T('\0');
}

CalculatorCommand::~CalculatorCommand()
{
	delete in;
}

void CalculatorCommand::SetResult(const CString& result)
{
	in->mResult = result;

	in->mDescription = result;
	in->mDescription += _T("\t(Enterでコピー)");
}

CString CalculatorCommand::GetName()
{
	return in->mResult;
}

CString CalculatorCommand::GetDescription()
{
	return in->mDescription;
}

BOOL CalculatorCommand::Execute()
{
	Parameter emptyParams;
	return Execute(emptyParams);
}

BOOL CalculatorCommand::Execute(const Parameter& param)
{
	// Calculatorといいつつ、ここに処理が及ぶ時点で計算はおわっていて、
	// ここでは単にクリップボードに結果をコピーするのみ

	// クリップボードにコピー
	size_t bufLen = sizeof(TCHAR) * (in->mResult.GetLength() + 1);
	HGLOBAL hMem = GlobalAlloc(GHND | GMEM_SHARE , bufLen);
	LPTSTR p = (LPTSTR)GlobalLock(hMem);
	_tcscpy_s(p, bufLen, in->mResult);
	GlobalUnlock(hMem);

	BOOL isSet=FALSE;
	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 9, (WPARAM)&isSet, (LPARAM)hMem);

	if (isSet == FALSE) {
		GlobalFree(hMem);
	}

	return TRUE;
}

CString CalculatorCommand::GetErrorString()
{
	return _T("");
}

HICON CalculatorCommand::GetIcon()
{
	if (in->mCalcPath[0] == _T('\0')) {
		size_t reqLen = 0;
		_tgetenv_s(&reqLen, in->mCalcPath, MAX_PATH_NTFS, _T("SystemRoot"));
		PathAppend(in->mCalcPath, _T("System32"));
		PathAppend(in->mCalcPath, _T("calc.exe"));
	}
	return IconLoader::Get()->LoadIconFromPath(in->mCalcPath);
}

int CalculatorCommand::Match(Pattern* pattern)
{
	// サポートしない
	return Pattern::WholeMatch;
}

bool CalculatorCommand::IsEditable()
{
	return false;
}

int CalculatorCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command*
CalculatorCommand::Clone()
{
	auto clonedObj = new CalculatorCommand();
	clonedObj->in->mResult = in->mResult;
	clonedObj->in->mDescription = in->mDescription;
	return clonedObj;
}

bool CalculatorCommand::Save(CommandFile* cmdFile)
{
	// 非サポート
	return false;
}

uint32_t CalculatorCommand::AddRef()
{
	return ++(in->mRefCount);
}

uint32_t CalculatorCommand::Release()
{
	auto n = --(in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace calculator
} // end of namespace commands
} // end of namespace soyokaze

