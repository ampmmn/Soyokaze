#include "pch.h"
#include "framework.h"
#include "CalculatorCommand.h"
#include "commands/common/Clipboard.h"
#include "icon/IconLoader.h"
#include "utility/Path.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace calculator {

constexpr LPCTSTR TYPENAME = _T("CalculatorCommand");

struct CalculatorCommand::PImpl
{
	// 基数(0以外の値が指定されたら基数を表示する)
	int mBase = 0;
	// 基数込みで表示する場合の種別名
	CString mTypeDisplayName;
	// 計算結果
	CString mResult;
	// アイコン取得用のファイルパス(calc.exeのアイコンを使う)
	Path mCalcPath;
};


CalculatorCommand::CalculatorCommand() : in(std::make_unique<PImpl>())
{
}

CalculatorCommand::CalculatorCommand(int base) : in(std::make_unique<PImpl>())
{
	ASSERT(base == 2 || base == 8 || base == 10 || base == 16);
	in->mBase = base;
}

CalculatorCommand::~CalculatorCommand()
{
}

void CalculatorCommand::SetResult(const CString& result)
{
	in->mResult = result;

	this->mName = result;
	this->mDescription = result;
}

CString CalculatorCommand::GetGuideString()
{
	return _T("Enter:クリップボードにコピー");
}

/**
 * 種別を表す文字列を取得する
 * @return 文字列
 */
CString CalculatorCommand::GetTypeName()
{
	return TYPENAME;
}

CString CalculatorCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_CALCULATOR);

	if (in->mBase == 0) {
		return TEXT_TYPE;
	}
	else {
		if (in->mTypeDisplayName.IsEmpty()) {
			in->mTypeDisplayName.Format(_T("%s (%d進数)"), (LPCTSTR)TEXT_TYPE, in->mBase);
		}
		return in->mTypeDisplayName;
	}
}

BOOL CalculatorCommand::Execute(const Parameter& param)
{
	UNREFERENCED_PARAMETER(param);

	// Calculatorといいつつ、ここに処理が及ぶ時点で計算はおわっていて、
	// ここでは単にクリップボードに結果をコピーするのみ

	// クリップボードにコピー
	Clipboard::Copy(in->mResult);

	return TRUE;
}

HICON CalculatorCommand::GetIcon()
{
	if (in->mCalcPath.IsEmptyPath()) {
		if (GetCalcExePath(in->mCalcPath,  in->mCalcPath.size()) == false) {
			// パスを取得できなかった場合(普通ないはず..)
			return IconLoader::Get()->LoadDefaultIcon(); 
		};
		in->mCalcPath.Shrink();
	}
	return IconLoader::Get()->LoadIconFromPath((LPCTSTR)in->mCalcPath);
}

launcherapp::core::Command*
CalculatorCommand::Clone()
{
	auto clonedObj = std::make_unique<CalculatorCommand>();
	clonedObj->SetResult(in->mResult);
	return clonedObj.release();
}

bool CalculatorCommand::GetCalcExePath(LPTSTR path, size_t len)
{
	size_t reqLen = 0;
	_tgetenv_s(&reqLen, path, len, _T("SystemRoot"));
	if  (len <= reqLen + 18) {   // 18: len("System32\calc.exe")
		return false;
	}
	PathAppend(path, _T("System32"));
	PathAppend(path, _T("calc.exe"));

	return true;
}

} // end of namespace calculator
} // end of namespace commands
} // end of namespace launcherapp

