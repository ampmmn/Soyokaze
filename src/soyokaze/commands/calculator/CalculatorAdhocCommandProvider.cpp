#include "pch.h"
#include "CalculatorAdhocCommandProvider.h"
#include "commands/calculator/CalculatorCommand.h"
#include "commands/calculator/Calculator.h"
#include "commands/core/CommandRepository.h"
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

	// 10進数で結果表示用のコマンド
	CalculatorCommand* mDecResultPtr{nullptr};
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
	in->mDecResultPtr = new CalculatorCommand();
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
	if (in->mDecResultPtr) {
		in->mDecResultPtr->Release();
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

	bool isBuiltinFunction = result.GetLength() > 0 && result[0] == _T('<');
	if (isBuiltinFunction) {
		return;
	}

	// 10進数としての結果を追加
	in->mDecResultPtr->SetResult(result);

	static tregex regexInt(_T("^-?[0-9]+$"));
	if (std::regex_match(tstring(result), regexInt) == false) {
		// 評価結果が整数値でない場合は、10進数の結果のみを表示
		in->mDecResultPtr->AddRef();
		commands.Add(CommandQueryItem(Pattern::FrontMatch, in->mDecResultPtr));
		return ;
	}
	// もし、評価結果が整数値なら16/8/2進数の結果も表示する

	// 先頭の要素が10進数/16進数/8進数/2進数かで先に表示する進数を替える
	static const int defaultOrder[] = { 10, 16, 8, 2};

	const int* order = defaultOrder;

	std::wstring cmdline_(cmdline);
	static tregex regHex(_T("^ *0x"));
	static tregex regOct(_T("^ *0o"));
	static tregex regBin(_T("^ *0b"));

	if (std::regex_search(cmdline_, regHex)) {
		// 16進数を先に表示
		static const int hexFirstOrder[] = { 16, 10, 8, 2};
		order = hexFirstOrder;
	}
	else if (std::regex_search(cmdline_, regOct) != false) {
		// 8進数を先に表示
		static const int octFirstOrder[] = { 8, 10, 16, 2};
		order = octFirstOrder;
	}
	else if (std::regex_search(cmdline_, regBin) != false) {
		// 2進数を先に表示
		static const int binFirstOrder[] = { 2, 10, 16, 8};
		order = binFirstOrder;
	}

	CString numStr(result);

	CString fmtStr;

	// 16進数変換
	fmtStr.Format(_T("hex(%s)"), (LPCTSTR)numStr);
	if (in->mCalc.Evaluate(fmtStr, result)) {
		result.Replace(_T("'"), _T(""));
		in->mHexResultPtr->SetResult(result);
	}

	// 8進数変換
	fmtStr.Format(_T("oct(%s)"), (LPCTSTR)numStr);
	if (in->mCalc.Evaluate(fmtStr, result)) {
		result.Replace(_T("'"), _T(""));
		in->mOctResultPtr->SetResult(result);
	}

	// 2進数変換
	fmtStr.Format(_T("bin(%s)"), (LPCTSTR)numStr);
	if (in->mCalc.Evaluate(fmtStr, result)) {
		result.Replace(_T("'"), _T(""));
		in->mBinResultPtr->SetResult(result);
	}

	// orderの順序に従って結果を格納する
	for (int i = 0; i < 4; ++i) {
		switch(order[i]) {
		case 2:
			in->mBinResultPtr->AddRef();
			commands.Add(CommandQueryItem(Pattern::FrontMatch, in->mBinResultPtr));
			break;
		case 8:
			in->mOctResultPtr->AddRef();
			commands.Add(CommandQueryItem(Pattern::FrontMatch, in->mOctResultPtr));
			break;
		case 10:
			in->mDecResultPtr->AddRef();
			commands.Add(CommandQueryItem(Pattern::FrontMatch, in->mDecResultPtr));
			break;
		case 16:
			in->mHexResultPtr->AddRef();
			commands.Add(CommandQueryItem(Pattern::FrontMatch, in->mHexResultPtr));
			break;
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

