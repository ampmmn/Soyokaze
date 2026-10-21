#include "pch.h"
#include "MacroRepository.h"
#include "macros/core/Token.h"
#include "utility/CommandLineArgument.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace macros {
namespace core {

namespace {

// マクロの展開結果は必要に応じてクォートして返す
// @param[in] text 解析対象の文字列
// @param[in] startPos マクロの記述の開始位置
// @param[in] endPos マクロの記述の終了位置の次
// @param[in] value マクロの展開結果
// @param[in] quoteValues クォートするかどうか
// @return 必要に応じてクォートされた文字列
CString FormatMacroValue(const CString& text, int startPos, int endPos, const CString& value, bool quoteValues)
{
	if (quoteValues == false || value.IsEmpty()) {
		return value;
	}

	// マクロの記述がダブルクォートで囲まれている場合は内側だけエスケープする
	bool isQuoted = (startPos > 0) && (endPos < text.GetLength()) &&
		(text[startPos - 1] == _T('"')) && (text[endPos] == _T('"'));
	if (isQuoted) {
		return launcherapp::utility::EscapeArgumentValue(value);
	}
	return launcherapp::utility::QuoteArgument(value);
}

} // namespace

struct MacroRepository::PImpl
{
	MacroIF* FindMacro(const CString& name)
	{
		auto it = mMacros.find(name);
		if (it == mMacros.end()) {
			return nullptr;
		}
		return it->second;
	}

	CCriticalSection mCS;
	std::map<CString, MacroIF*> mMacros;
};


MacroRepository::MacroRepository() : in(std::make_unique<PImpl>())
{
}

MacroRepository::~MacroRepository()
{
	ReleaseAllMacros();
}

MacroRepository* MacroRepository::GetInstance()
{
	static MacroRepository inst;
	return &inst;
}

void MacroRepository::ReleaseAllMacros()
{
	CSingleLock sl(&in->mCS, TRUE);

	for (auto& item : in->mMacros) {
		auto macro = item.second;
		macro->Release();
	}
	in->mMacros.clear();
}

// 登録
bool MacroRepository::RegisterMacro(
	MacroIF* macro
)
{
	ASSERT(macro);

	CSingleLock sl(&in->mCS, TRUE);


	auto name = macro->GetName();
	auto itFind = in->mMacros.find(name);
	if (itFind != in->mMacros.end()) {
		auto orgMacro = itFind->second;
		orgMacro->Release();
	}

	in->mMacros[name] = macro;
	macro->AddRef();

	return true;
}

bool MacroRepository::UnregisterMacro(MacroIF* macro)
{
	ASSERT(macro);

	CSingleLock sl(&in->mCS, TRUE);

	auto it = in->mMacros.find(macro->GetName());
	if (it == in->mMacros.end()) {
		return false;
	}

	it->second->Release();
	in->mMacros.erase(it);
	return true;
}

// 文字列を解析して置換する
bool MacroRepository::Evaluate(CString& text, bool quoteValues)
{
	try {
		CString output;

		Token token(text);
		while (token.AtEnd() == false) {

			if (token.Get() == _T('\\')) {
				// エスケープ
				token.Next(); // '\'をスキップ

				if (token.AtEnd()) {
					output += _T('\\');
					break;
				}

				if (token.Get() != _T('$')) {
					output += _T('\\');
					continue;
				}

				output += token.Get();
				token.Next(); // '\'直後の'$'もスキップ
				continue;
			}
			if (token.Get() != _T('$')) {
				// マクロ記述ではない
				output += token.Get();
				token.Next(); // '\'をスキップ
				continue;
			}

			// 以後、マクロ記述の処理

			// マクロの記述の開始位置(クォート済みかどうかを判定するために使う)
			int macroStartPos = token.GetPos();

			token.Next();   // $をスキップ

			if (token.Get() != _T('{')) {

				// $xxx の記法
				CString name;
				token.SkipName(name);

				MacroIF* macro = in->FindMacro(name); 
				if (macro == nullptr) {
					SPDLOG_WARN(_T("macro name:'{}' does not found."), (LPCTSTR)name);
					output += _T('$');
					output += name;
					continue;
				}

				// $xxx 形式のマクロを呼び出す
				CString macroValue;

				std::vector<CString> emptyArg;
				macro->Evaluate(emptyArg, macroValue);

				output += FormatMacroValue(text, macroStartPos, token.GetPos(), macroValue, quoteValues);
				continue;
			}

			// ${...} の記法
			token.Next();     // '{'をスキップ

			token.SkipWhiteSpace();   // 空白文字をスキップ

			CString name;
			token.SkipName(name);

			if (token.Get() != _T('}') && token.IsWhiteSpace() == false) {
				SPDLOG_WARN(_T("Invalid char. pos:{}"), token.GetPos());
				token.SkipUntil(_T('}'));   // '}'までスキップ
				token.Next();   // '}' をスキップ
				continue;
			}

			token.SkipWhiteSpace();   // 名前直後のスペースをスキップ

			std::vector<CString> args;
			while (token.Get() != _T('}')) {
				CString arg;
				token.SkipString(arg);
				args.push_back(arg);

				token.SkipWhiteSpace();
			}
			token.Next();   // '}'をスキップ

			MacroIF* macro = in->FindMacro(name); 
			if (macro == nullptr) {
				SPDLOG_WARN(_T("macro name:'{}' does not found."), (LPCTSTR)name);
				continue;
			}

			// ${xxx} 形式のマクロを呼び出す
			CString macroValue;
			macro->Evaluate(args, macroValue);

			output += FormatMacroValue(text, macroStartPos, token.GetPos(), macroValue, quoteValues);
		}

		text = output;
		return true;
	}
	catch(Token::Exception&) {
		return false;
	}
}

}
}
}

