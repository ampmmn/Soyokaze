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

	uint32_t mRefCount;

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
	in->mRefCount = 1;
	in->mCommandPtr = new CalculatorCommand();
}

CalculatorAdhocCommandProvider::~CalculatorAdhocCommandProvider()
{
	if (in->mCommandPtr) {
		in->mCommandPtr->Release();
	}
}

// 初回起動の初期化を行う
void CalculatorAdhocCommandProvider::OnFirstBoot()
{
	// 何もしない
}


// コマンドの読み込み
void CalculatorAdhocCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	// 何もしない
}

CString CalculatorAdhocCommandProvider::GetName()
{
	return _T("Calculator");
}

// 作成できるコマンドの種類を表す文字列を取得
CString CalculatorAdhocCommandProvider::GetDisplayName()
{
	// サポートしない
	return _T("");
}

// コマンドの種類の説明を示す文字列を取得
CString CalculatorAdhocCommandProvider::GetDescription()
{
	// サポートしない
	return _T("");
}

// コマンド新規作成ダイアログ
bool CalculatorAdhocCommandProvider::NewDialog(const CommandParameter* param)
{
	// サポートしない
	return false;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool CalculatorAdhocCommandProvider::IsPrivate() const
{
	return true;
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

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t CalculatorAdhocCommandProvider::CalculatorAdhocCommandProvider::GetOrder() const
{
	return 1500;
}

uint32_t CalculatorAdhocCommandProvider::CalculatorAdhocCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t CalculatorAdhocCommandProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace calculator
} // end of namespace commands
} // end of namespace soyokaze

