#pragma once

#include <vector>

namespace launcherapp {
namespace macros {
namespace core {

class MacroIF
{
public:
	virtual ~MacroIF() {}

	virtual CString GetName() = 0;
	virtual CString GetDescription() = 0;
	// 評価結果を0個以上の値として返す。
	virtual bool Evaluate(const std::vector<CString>& args, std::vector<CString>& results) = 0;

	virtual uint32_t AddRef() = 0;
	virtual uint32_t Release() = 0;

};

}
} // end of namespace core
} // end of namespace launcherapp

