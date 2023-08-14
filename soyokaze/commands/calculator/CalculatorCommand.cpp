#include "pch.h"
#include "framework.h"
#include "CalculatorCommand.h"
#include "commands/common/ExecuteHistory.h"
#include "utility/GlobalAllocMemory.h"
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
	TCHAR mCalcPath[MAX_PATH_NTFS];
};


CalculatorCommand::CalculatorCommand() : in(new PImpl)
{
	in->mCalcPath[0] = _T('\0');
}

CalculatorCommand::~CalculatorCommand()
{
}

void CalculatorCommand::SetResult(const CString& result)
{
	in->mResult = result;

	// 基底クラスのメンバー変数で持つ名前と説明を設定する
	this->mName = result;
	this->mDescription = result;
	this->mDescription += _T("\t(Enterでコピー)");
}

CString CalculatorCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_CALCULATOR);
	return TEXT_TYPE;
}

BOOL CalculatorCommand::Execute(const Parameter& param)
{
	// Calculatorといいつつ、ここに処理が及ぶ時点で計算はおわっていて、
	// ここでは単にクリップボードに結果をコピーするのみ

	// クリップボードにコピー
	size_t bufLen = sizeof(TCHAR) * (in->mResult.GetLength() + 1);
	GlobalAllocMemory mem(bufLen);
	_tcscpy_s((LPTSTR)mem.Lock(), bufLen, in->mResult);
	mem.Unlock();

	BOOL isSet=FALSE;
	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 9, 
	            (WPARAM)&isSet, (LPARAM)(HGLOBAL)mem);

	if (isSet) {
		// コピーが実施されたら、GlobalAllocの所有権を放棄する
		mem.Release();
	}

	return TRUE;
}

HICON CalculatorCommand::GetIcon()
{
	if (in->mCalcPath[0] == _T('\0')) {
		if (GetCalcExePath(in->mCalcPath,  MAX_PATH_NTFS) == false) {
			// パスを取得できなかった場合(普通ないはず..)
			return IconLoader::Get()->LoadDefaultIcon(); 
		};
	}
	return IconLoader::Get()->LoadIconFromPath(in->mCalcPath);
}

soyokaze::core::Command*
CalculatorCommand::Clone()
{
	auto clonedObj = new CalculatorCommand();
	clonedObj->SetResult(in->mResult);
	return clonedObj;
}

bool CalculatorCommand::GetCalcExePath(LPTSTR path, size_t len)
{
	size_t reqLen = 0;
	_tgetenv_s(&reqLen, path, MAX_PATH_NTFS, _T("SystemRoot"));
	if  (len <= reqLen + 18) {
		return false;
	}
	PathAppend(path, _T("System32"));
	PathAppend(path, _T("calc.exe"));

	return true;
}

} // end of namespace calculator
} // end of namespace commands
} // end of namespace soyokaze

