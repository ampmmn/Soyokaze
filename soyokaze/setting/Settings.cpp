#include "pch.h"
#include "setting/Settings.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct Settings::PImpl
{
	std::set<CString> mKeys;

	std::map<CString, int> mIntMap;
	std::map<CString, CString> mStrMap;
	std::map<CString, bool> mBoolMap;
	std::map<CString, double> mDoubleMap;
};


Settings::Settings() : in(std::make_unique<PImpl>())
{
}

Settings::~Settings()
{
}

void Settings::EnumKeys(std::set<CString>& keys) const
{
	keys = in->mKeys;
}

int Settings::GetType(LPCTSTR key) const
{
	if (in->mIntMap.count(key) != 0) {
		return TYPE_INT;
	}
	if (in->mDoubleMap.count(key) != 0) {
		return TYPE_DOUBLE;
	}
	if (in->mStrMap.count(key) != 0) {
		return TYPE_STRING;
	}
	if (in->mBoolMap.count(key) != 0) {
		return TYPE_BOOLEAN;
	}
	return TYPE_UNKNOWN;
}

bool Settings::Has(LPCTSTR key) const
{
	return in->mKeys.find(key) != in->mKeys.end();
}

int Settings::Get(LPCTSTR key, int defValue) const
{
	auto itFind = in->mIntMap.find(key);
	if (itFind == in->mIntMap.end()) {
		return defValue;
	}
	return itFind->second;
}

void Settings::Set(LPCTSTR key, int value)
{
	auto itFind = in->mIntMap.find(key);
	if (itFind != in->mIntMap.end()) {
		itFind->second = value;
		return;
	}

	if (in->mKeys.find(key) != in->mKeys.end()) {
		// 他の型でキーが存在する
		throw Exception();
	}

	in->mIntMap[key] = value;
	in->mKeys.insert(key);
}


double Settings::Get(LPCTSTR key, double defValue) const
{
	auto itFind = in->mDoubleMap.find(key);
	if (itFind == in->mDoubleMap.end()) {
		return defValue;
	}
	return itFind->second;
}

void Settings::Set(LPCTSTR key, double value)
{
	auto itFind = in->mDoubleMap.find(key);
	if (itFind != in->mDoubleMap.end()) {
		itFind->second = value;
		return;
	}

	if (in->mKeys.find(key) != in->mKeys.end()) {
		// 他の型でキーが存在する
		throw Exception();
	}

	in->mDoubleMap[key] = value;
	in->mKeys.insert(key);
}


CString Settings::Get(LPCTSTR key, LPCTSTR defValue) const
{
	auto itFind = in->mStrMap.find(key);
	if (itFind == in->mStrMap.end()) {
		return defValue;
	}
	return itFind->second;
}

void Settings::Set(LPCTSTR key, const CString& value)
{
	auto itFind = in->mStrMap.find(key);
	if (itFind != in->mStrMap.end()) {
		itFind->second = value;
		return;
	}

	if (in->mKeys.find(key) != in->mKeys.end()) {
		// 他の型でキーが存在する
		throw Exception();
	}

	in->mStrMap[key] = value;
	in->mKeys.insert(key);
}


bool Settings::Get(LPCTSTR key, bool defValue) const
{
	auto itFind = in->mBoolMap.find(key);
	if (itFind == in->mBoolMap.end()) {
		return defValue;
	}
	return itFind->second;
}

void Settings::Set(LPCTSTR key, bool value)
{
	auto itFind = in->mBoolMap.find(key);
	if (itFind != in->mBoolMap.end()) {
		itFind->second = value;
		return;
	}

	if (in->mKeys.find(key) != in->mKeys.end()) {
		// 他の型でキーが存在する
		throw Exception();
	}

	in->mBoolMap[key] = value;
	in->mKeys.insert(key);
}


Settings* Settings::Clone() const
{
	auto newObj = std::make_unique<Settings>();
	newObj->in->mKeys = in->mKeys;
	newObj->in->mIntMap = in->mIntMap;
	newObj->in->mStrMap = in->mStrMap;
	newObj->in->mBoolMap = in->mBoolMap;
	newObj->in->mDoubleMap = in->mDoubleMap;

	return newObj.release();
}

void Settings::Swap(Settings& rhs)
{
	in->mKeys.swap(rhs.in->mKeys);
	in->mIntMap.swap(rhs.in->mIntMap);
	in->mStrMap.swap(rhs.in->mStrMap);
	in->mBoolMap.swap(rhs.in->mBoolMap);
	in->mDoubleMap.swap(rhs.in->mDoubleMap);
}



