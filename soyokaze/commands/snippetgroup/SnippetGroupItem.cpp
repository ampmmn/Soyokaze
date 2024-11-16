#include "pch.h"
#include "SnippetGroupItem.h"
#include "commands/core/CommandEntryIF.h"

#ifndef UNICODE
#error TCHAR must be wchar_t.
#endif

namespace launcherapp {
namespace commands {
namespace snippetgroup {

Item::Item()
{
}

Item::~Item()
{
}

bool Item::operator == (const Item& rhs) const
{
	return mName == rhs.mName && mDescription == rhs.mDescription && mText == rhs.mText;
}

bool Item::Save(CommandEntryIF* entry, LPCTSTR prefix, utility::aes::AES& aes)
{
	CString keyName;
	keyName.Format(_T("%s_Name"), prefix);
	entry->Set(keyName, mName);
	keyName.Format(_T("%s_Description"), prefix);
	entry->Set(keyName, mDescription);

	std::vector<uint8_t> plainData;
	plainData.resize((mText.GetLength()+1) * sizeof(wchar_t));
	memcpy(plainData.data(), (LPCTSTR)mText, plainData.size());

	std::vector<uint8_t> cryptData;
	aes.Encrypt(plainData, cryptData);

	keyName.Format(_T("%s_Text"), prefix);
	entry->SetBytes(keyName, cryptData.data(), cryptData.size());

	return true;
}

bool Item::Load(CommandEntryIF* entry, LPCTSTR prefix, utility::aes::AES& aes)
{
	CString keyName;
	keyName.Format(_T("%s_Name"), prefix);
	mName = entry->Get(keyName, _T(""));
	keyName.Format(_T("%s_Description"), prefix);
	mDescription = entry->Get(keyName, _T(""));
	keyName.Format(_T("%s_Text"), prefix);

	int textVlueType = entry->GetValueType(keyName);
	if (textVlueType == 2) { 
		mText = entry->Get(keyName, _T(""));
		return true;
	}
	else if (textVlueType == 4) {
		size_t len = entry->GetBytesLength(keyName);
		std::vector<uint8_t> cryptData(len);
		if (entry->GetBytes(keyName, cryptData.data(), len) == false) {
			spdlog::error(_T("Failed to get value : {}"), (LPCTSTR)keyName);
			return false;
		}
		std::vector<uint8_t> plainData;
		if (aes.Decrypt(cryptData, plainData) == false) {
			spdlog::error(_T("Failed to decrypt : {}"), (LPCTSTR)keyName);
			return false;
		}
		mText = (LPCTSTR)plainData.data();
		return true;
	}

	// その他の型
	spdlog::error(_T("Unknown data type: {}"), (LPCTSTR)keyName);
	return false;
}

void Item::swap(Item& rhs)
{
	std::swap(mName, rhs.mName);
	std::swap(mDescription, rhs.mDescription);
	std::swap(mText, rhs.mText);
}

}
}
}

