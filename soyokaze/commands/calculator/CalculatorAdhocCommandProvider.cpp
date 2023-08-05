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

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(CalculatorAdhocCommandProvider)


CalculatorAdhocCommandProvider::CalculatorAdhocCommandProvider() : in(new PImpl)
{
	in->mIsFirstCall = true;
	in->mIsEnable = false;
	in->mCommandPtr = new CalculatorCommand();
}

CalculatorAdhocCommandProvider::~CalculatorAdhocCommandProvider()
{
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
 	std::vector<CommandQueryItem>& commands
)
{
	if (in->mIsFirstCall) {
		auto pref = AppPreference::Get();
		in->mCalc.SetPythonDLLPath(pref->GetPythonDLLPath());
		in->mIsEnable = pref->IsEnableCalculator();
		in->mIsFirstCall = false;
	}
	CString cmdline = pattern->GetOriginalPattern();


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
}


} // end of namespace calculator
} // end of namespace commands
} // end of namespace soyokaze

