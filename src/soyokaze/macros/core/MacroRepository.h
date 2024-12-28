#pragma once

#include "macros/core/MacroIF.h"
#include <memory>

namespace launcherapp {
namespace macros {
namespace core {

class MacroRepository
{
private:
	MacroRepository();
	virtual ~MacroRepository();

public:
	static MacroRepository* GetInstance();
	void ReleaseAllMacros();

	// マクロ登録
	bool RegisterMacro(MacroIF* macro);
	bool UnregisterMacro(MacroIF* macro);

	// 文字列を解析して置換する
	bool Evaluate(CString& text);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}
}

// 派生クラス側で下記のマクロを通じてCommandProviderとして登録する

#define DECLARE_LAUNCHERMACRO(clsName)   static bool RegisterMacro(); \
	private: \
	static bool _mIsMacroRegistered; \
	public: \
	static bool IsMacroRegistered();

#define REGISTER_LAUNCHERMACRO(clsName)   \
	bool clsName::RegisterMacro() { \
		try { \
			clsName* inst = new clsName(); \
			launcherapp::macros::core::MacroRepository::GetInstance()->RegisterMacro(inst); \
			inst->Release(); \
			return true; \
		} catch(...) { return false; } \
	} \
	bool clsName::_mIsMacroRegistered = clsName::RegisterMacro(); \
	bool clsName::IsMacroRegistered() { return _mIsMacroRegistered; }


