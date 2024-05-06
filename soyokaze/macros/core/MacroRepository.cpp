#include "pch.h"
#include "MacroRepository.h"
#include "macros/core/Token.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace macros {
namespace core {

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
bool MacroRepository::Evaluate(CString& text)
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

			token.Next();   // $をスキップ

			if (token.Get() != _T('{')) {

				// $xxx の記法
				CString name;
				token.SkipName(name);

				MacroIF* macro = in->FindMacro(name); 
				if (macro == nullptr) {
					SPDLOG_WARN(_T("macro {} does not found."), (LPCTSTR)name);
					continue;
				}

				// $xxx 形式のマクロを呼び出す
				CString macroValue;

				std::vector<CString> emptyArg;
				macro->Evaluate(emptyArg, macroValue);

				output += macroValue;
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
				SPDLOG_WARN(_T("macro {} does not found."), (LPCTSTR)name);
				continue;
			}

			// ${xxx} 形式のマクロを呼び出す
			CString macroValue;
			macro->Evaluate(args, macroValue);

			output += macroValue;
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

