#include "pch.h"
#include "Honyaku.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "utility/CharConverter.h"
#include <nlohmann/json.hpp>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace core {

using json = nlohmann::json;

struct Honyaku::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}
	int EnumLangCodes(std::vector<LangCode>& codes);
	bool Reload();

	virtual void OnAppFirstBoot()
	{
	}
	virtual void OnAppPreferenceUpdated()
	{
		mShouldReload = true;
	}

	virtual void OnAppExit()
	{
	}

	static BOOL CALLBACK OnEnumChildWindows(HWND h, LPARAM lp);
	BOOL Apply(HWND h);

	std::map<CString, CString> mCurrentLangMap;
	bool mShouldReload;
};

int Honyaku::PImpl::EnumLangCodes(std::vector<LangCode>& codes)
{
	TCHAR path[MAX_PATH_NTFS];
	GetModuleFileName(nullptr, path, MAX_PATH_NTFS);
	PathRemoveFileSpec(path);
	PathAppend(path, _T("lang"));
	PathAppend(path, _T("lang.json"));

	std::vector<LangCode> tmp;

	utility::CharConverter conv;
	try {
		std::ifstream f(path);
		json langs = json::parse(f);
		for (auto it = langs.begin(); it != langs.end(); ++it) {
			LangCode item;
			std::string val = it.key();
			conv.Convert(val.c_str(), item.mCode);
			val = it.value();
			conv.Convert(val.c_str(), item.mDisplayName);
			tmp.push_back(item);
		}

		codes.swap(tmp);
		return 0;
	}
	catch(std::exception&) {
		return 1;
	}
}

bool Honyaku::PImpl::Reload()
{
	CString langCode = AppPreference::Get()->GetLangCode();

	if (langCode.IsEmpty()) {
		if (GetACP() == 932) {
			langCode = _T("ja");
		}
		else {
			langCode = _T("en");
		}
	}

	TCHAR path[MAX_PATH_NTFS];
	GetModuleFileName(nullptr, path, MAX_PATH_NTFS);
	PathRemoveFileSpec(path);
	PathAppend(path, _T("lang"));
	PathAppend(path, langCode);
	PathAddExtension(path, _T(".json"));
	if (PathFileExists(path) == FALSE) {
		return false;
	}

	std::map<CString, CString> langMap;

	utility::CharConverter conv;
	CString key;
	CString value;
	try {
		std::ifstream f(path);
		json langs = json::parse(f);
		for (auto it = langs.begin(); it != langs.end(); ++it) {
			std::string val = it.key();
			conv.Convert(val.c_str(), key);
			val = it.value();
			conv.Convert(val.c_str(), value);
			langMap[key] = value;
		}
		mCurrentLangMap.swap(langMap);
		return true;
	}
	catch(std::exception&) {
		return false;
	}
	return false;
}

BOOL Honyaku::PImpl::Apply(HWND h)
{
	TCHAR caption[65535];
	GetWindowText(h, caption, 65535);

	TCHAR clsName[256];
	GetClassName(h, clsName, 256);

	// ComBoBox用の処理
	if (_tcscmp(clsName, _T("ComboBox")) == 0) {
		CComboBox* p = (CComboBox*)CComboBox::FromHandle(h);
		ASSERT(p);

		int n = p->GetCount();
		for (int i = 0; i < n; ++i) {
			CString str;
			p->GetLBText(i, str);

			auto it = mCurrentLangMap.find(str);
			if (it !=  mCurrentLangMap.end()) {
				p->DeleteString(i);
				p->InsertString(i, it->second);
			}
		}

	}

	auto it = mCurrentLangMap.find(caption);
	if (it !=  mCurrentLangMap.end()) {
		SetWindowText(h, it->second);
	}
	return TRUE;
}

BOOL Honyaku::PImpl::OnEnumChildWindows(HWND h, LPARAM lp)
{
	PImpl* in = (PImpl*)lp;
	in->Apply(h);

	return TRUE;
}

Honyaku::Honyaku() : in(new PImpl)
{
	in->mShouldReload = true;
}

Honyaku::~Honyaku()
{
}

Honyaku* Honyaku::Get()
{
	static Honyaku inst;
	return &inst;
}

CString& Honyaku::Str(CString& str)
{
	if (in->mShouldReload) {
		in->Reload();
		in->mShouldReload = false;
	}
	auto it = in->mCurrentLangMap.find(str);
	if (it == in->mCurrentLangMap.end()) {
		str = it->second;
	}
	return str;
}

CString Honyaku::Str(const CString& str, LPCTSTR def)
{
	if (in->mShouldReload) {
		in->Reload();
		in->mShouldReload = false;
	}
	auto it = in->mCurrentLangMap.find(str);
	if (it == in->mCurrentLangMap.end()) {
		return def;
	}
	return it->second;
}

CString Honyaku::Literal(LPCTSTR text)
{
	if (in->mShouldReload) {
		in->Reload();
		in->mShouldReload = false;
	}
	auto it = in->mCurrentLangMap.find(text);
	if (it == in->mCurrentLangMap.end()) {
		return text;
	}
	return it->second;
}

void Honyaku::Hwnd(HWND hwnd, bool isIncludeChildren)
{
	if (in->mShouldReload) {
		in->Reload();
		in->mShouldReload = false;
	}

	in->Apply(hwnd);

	EnumChildWindows(hwnd, PImpl::OnEnumChildWindows, (LPARAM)in.get());

}


int Honyaku::EnumLangCodes(std::vector<LangCode>& codes)
{
	return in->EnumLangCodes(codes);
}

}
}

