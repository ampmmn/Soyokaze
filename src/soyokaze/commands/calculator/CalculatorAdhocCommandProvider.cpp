#include "pch.h"
#include "CalculatorAdhocCommandProvider.h"
#include "commands/calculator/CalculatorCommand.h"
#include "commands/calculator/Calculator.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace calculator {


using CommandRepository = launcherapp::core::CommandRepository;

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
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mIsEnable = pref->IsEnableCalculator();
	}
	void OnAppExit() override {}

	bool mIsEnable{true};

	//
	Calculator mCalc;

	// 環境変数PATHにあるexeを実行するためのコマンド
	CalculatorCommand* mCommandPtr{nullptr};
	// 16進数で結果表示用のコマンド
	CalculatorCommand* mHexResultPtr{nullptr};
	// 8進数で結果表示用のコマンド
	CalculatorCommand* mOctResultPtr{nullptr};
	// 2進数で結果表示用のコマンド
	CalculatorCommand* mBinResultPtr{nullptr};

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(CalculatorAdhocCommandProvider)


CalculatorAdhocCommandProvider::CalculatorAdhocCommandProvider() : in(std::make_unique<PImpl>())
{
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

// 一時的なコマンドの準備を行うための初期化
void CalculatorAdhocCommandProvider::PrepareAdhocCommands()
{
	auto pref = AppPreference::Get();
	in->mIsEnable = pref->IsEnableCalculator();
}

// 一時的なコマンドを必要に応じて提供する
void CalculatorAdhocCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	CString cmdline = pattern->GetWholeString();


	// 機能が無効なら評価実施しない
	if (in->mIsEnable == false) {
		return;
	}
	
	CString result;
	if (in->mCalc.Evaluate(cmdline, result) == false) {
		return;
	}

	// 10進数としての結果を追加
	// ただし、入力文字列と結果が全く同じ場合は表示しない
	// 例: 1 -> 1 のようなケース
	bool isBuiltinFunction = result.GetLength() > 0 && result[0] == _T('<');
	if (isBuiltinFunction == false && cmdline != result) {
		in->mCommandPtr->SetResult(result);
		in->mCommandPtr->AddRef();
		commands.Add(CommandQueryItem(Pattern::PartialMatch, in->mCommandPtr));
	}

	// もし、評価結果が整数値なら16/8/2進数の結果も表示する
	static tregex regexInt(_T("^-?[0-9]+$"));
	if (std::regex_match(tstring(result), regexInt)) {
		CString numStr(result);

		CString fmtStr;

		// 16進数
		fmtStr.Format(_T("hex(%s)"), (LPCTSTR)numStr);
		if (in->mCalc.Evaluate(fmtStr, result)) {
			result.Replace(_T("'"), _T(""));
			in->mHexResultPtr->SetResult(result);
			in->mHexResultPtr->AddRef();
			commands.Add(CommandQueryItem(Pattern::PartialMatch, in->mHexResultPtr));
		}

		// 8
		fmtStr.Format(_T("oct(%s)"), (LPCTSTR)numStr);
		if (in->mCalc.Evaluate(fmtStr, result)) {
			result.Replace(_T("'"), _T(""));
			in->mOctResultPtr->SetResult(result);
			in->mOctResultPtr->AddRef();
			commands.Add(CommandQueryItem(Pattern::PartialMatch, in->mOctResultPtr));
		}

		// 2進数
		fmtStr.Format(_T("bin(%s)"), (LPCTSTR)numStr);
		if (in->mCalc.Evaluate(fmtStr, result)) {
			result.Replace(_T("'"), _T(""));
			in->mBinResultPtr->SetResult(result);
			in->mBinResultPtr->AddRef();
			commands.Add(CommandQueryItem(Pattern::PartialMatch, in->mBinResultPtr));
		}
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t CalculatorAdhocCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(CalculatorCommand::TypeDisplayName());
	return 1;
}


} // end of namespace calculator
} // end of namespace commands
} // end of namespace launcherapp

