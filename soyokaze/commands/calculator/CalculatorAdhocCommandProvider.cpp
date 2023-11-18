#include "pch.h"
#include "CalculatorAdhocCommandProvider.h"
#include "commands/calculator/CalculatorCommand.h"
#include "commands/calculator/Calculator.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "AppPreferenceListenerIF.h"
#include "AppPreference.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace calculator {


using CommandRepository = soyokaze::core::CommandRepository;

struct CalculatorAdhocCommandProvider::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void OnAppFirstBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mCalc.SetPythonDLLPath(pref->GetPythonDLLPath());
		mIsEnable = pref->IsEnableCalculator();
	}
	void OnAppExit() override {}

	bool mIsFirstCall;
	bool mIsEnable;

	//
	Calculator mCalc;

	// 環境変数PATHにあるexeを実行するためのコマンド
	CalculatorCommand* mCommandPtr;
	// 16進数で結果表示用のコマンド
	CalculatorCommand* mHexResultPtr;
	// 8進数で結果表示用のコマンド
	CalculatorCommand* mOctResultPtr;
	// 2進数で結果表示用のコマンド
	CalculatorCommand* mBinResultPtr;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(CalculatorAdhocCommandProvider)


CalculatorAdhocCommandProvider::CalculatorAdhocCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mIsFirstCall = true;
	in->mIsEnable = false;
	in->mCommandPtr = new CalculatorCommand();
	in->mHexResultPtr = new CalculatorCommand(16);
	in->mOctResultPtr = new CalculatorCommand(8);
	in->mBinResultPtr = new CalculatorCommand(2);
}

CalculatorAdhocCommandProvider::~CalculatorAdhocCommandProvider()
{
	if (in->mBinResultPtr) {
		in->mBinResultPtr->Release();
	}
	if (in->mOctResultPtr) {
		in->mOctResultPtr->Release();
	}
	if (in->mHexResultPtr) {
		in->mHexResultPtr->Release();
	}
	if (in->mCommandPtr) {
		in->mCommandPtr->Release();
	}
}

CString CalculatorAdhocCommandProvider::GetName()
{
	return _T("Calculator");
}

// 一時的なコマンドを必要に応じて提供する
void CalculatorAdhocCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	if (in->mIsFirstCall) {
		auto pref = AppPreference::Get();
		in->mCalc.SetPythonDLLPath(pref->GetPythonDLLPath());
		in->mIsEnable = pref->IsEnableCalculator();
		in->mIsFirstCall = false;
	}
	CString cmdline = pattern->GetWholeString();


	// 機能が無効なら評価実施しない
	if (in->mIsEnable == false) {
		return;
	}
	
	CString result;
	if (in->mCalc.Evaluate(cmdline, result) == false) {
		return;
	}

	in->mCommandPtr->SetResult(result);
	in->mCommandPtr->AddRef();
	commands.push_back(CommandQueryItem(Pattern::WholeMatch, in->mCommandPtr));

	// もし、評価結果が整数値なら16/8/2進数の結果も表示する
	static tregex regexInt(_T("^-?[0-9]+$"));
	if (std::regex_match(tstring(result), regexInt)) {
		int64_t n = _ttoi64(result);
		CString buf;

		// 16進数
		buf.Format(_T("0x%x"), n);
		in->mHexResultPtr->SetResult(buf);
		in->mHexResultPtr->AddRef();
		commands.push_back(CommandQueryItem(Pattern::WholeMatch, in->mHexResultPtr));

		// 8
		buf.Format(_T("0o%o"), n);
		in->mOctResultPtr->SetResult(buf);
		in->mOctResultPtr->AddRef();
		commands.push_back(CommandQueryItem(Pattern::WholeMatch, in->mOctResultPtr));

		// 2進数
		CString tmp;
		tmp.Format(_T("bin(%I64d)"), n);
		// printfで変換できないので、Pythonでやってもらう
		if (in->mCalc.Evaluate(tmp, result)) {
			// binの結果はStringなので、表示用に引用符を外す
			result.Replace(_T("'"), _T(""));
			in->mBinResultPtr->SetResult(result);
			in->mBinResultPtr->AddRef();
			commands.push_back(CommandQueryItem(Pattern::WholeMatch, in->mBinResultPtr));
		}
	}
}


} // end of namespace calculator
} // end of namespace commands
} // end of namespace soyokaze

