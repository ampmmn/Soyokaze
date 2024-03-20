#pragma once

#include <memory>
#include <vector>

namespace soyokaze {
namespace core {

class Honyaku
{
public:
	struct LangCode
	{
		CString mCode;
		CString mDisplayName;
	};
public:
	static Honyaku* Get();

private:
	Honyaku();
	~Honyaku();

public:
	CString& Str(CString& str);
	CString Str(const CString& str, LPCTSTR def = nullptr);
	CString Literal(LPCTSTR text);
	void Hwnd(HWND hwnd, bool isIncludeChildren);

	int EnumLangCodes(std::vector<LangCode>& codes);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

#define _LANG_T(val) soyokaze::core::Honyaku::Get()->Str(_T(val))
#define _LANG_WINDOW(hwnd) soyokaze::core::Honyaku::Get()->Hwnd(hwnd, true)

} // end of namespace core
} // end of namespace soyokaze
