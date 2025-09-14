#include "pch.h"
#include "OneNoteAppProxy.h"
#include "commands/common/AutoWrap.h"

#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.Foundation.Collections.h>

using namespace launcherapp::commands::common;
using namespace winrt;
using namespace winrt::Windows::Data::Xml::Dom;

namespace launcherapp { namespace commands { namespace onenote {

struct OneNoteAppProxy::PImpl
{
	bool InitializeApp() {

		if (!(mApp == nullptr)) {
			// 初期化済
			return true;
		}

		CLSID CLSID_OneNote;
		HRESULT hr = CLSIDFromProgID(L"OneNote.Application.12", &CLSID_OneNote);
		if (FAILED(hr)) {
			return false;
		}
		hr = CoCreateInstance(CLSID_OneNote, nullptr, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void**)&mApp);
		if (FAILED(hr)) {
			return false;
		}
		return true;
	}

	// OneNote.Application.GetHierarchyをよぶ
	bool CallGetHierarchy(std::wstring& hierarchyXml) {

		CComPtr<IDispatch>& oneNoteApp = mApp;

		VARIANT result;
		VariantInit(&result);

		CComBSTR xmlOutput(L"");

		VARIANT argXmlOutput;
		VariantInit(&argXmlOutput);
		argXmlOutput.vt = VT_BSTR | VT_BYREF;
		argXmlOutput.pbstrVal = &xmlOutput;

		VARIANT argHsScope;
		VariantInit(&argHsScope);
		argHsScope.vt = VT_INT;
		argHsScope.intVal = 4;

		VARIANT argStartNodeID;
		VariantInit(&argStartNodeID);
		argStartNodeID.vt = VT_BSTR;
		argStartNodeID.bstrVal = nullptr;

		HRESULT hr = AutoWrap(DISPATCH_METHOD, &result, oneNoteApp, L"GetHierarchy", 3, argXmlOutput, argHsScope, argStartNodeID);
		if (FAILED(hr)) {
			return false;
		}

		hierarchyXml = xmlOutput;
		return true;
	}

	bool ParseHierarchyXML(const std::wstring& xml, std::vector<OneNoteBook>& oneNoteBooks) {

		XmlDocument doc;

		try {
			doc.LoadXml(xml.c_str());
		} catch(const winrt::hresult_error& e) {
			auto msg = e.message();
			spdlog::error(L"Failed to parse XML code:{0} message:{1}", static_cast<uint32_t>(e.code()), msg.c_str());
			return false;
		}

		XmlElement elemNoteBooks = doc.DocumentElement();  // NoteBooks

		std::vector<OneNoteBook> tmpBooks;

		XmlNodeList noteBooks = elemNoteBooks.ChildNodes();
		for (uint32_t i = 0; i < noteBooks.Size(); ++i) {
			XmlElement elemNoteBook = noteBooks.GetAt(i).as<XmlElement>();
			auto tagName = elemNoteBook.TagName();
			if (tagName != L"one:Notebook") {
				continue;
			}

			OneNoteBook oneNoteBook {
				elemNoteBook.GetAttribute(L"ID").c_str(),
				elemNoteBook.GetAttribute(L"name").c_str(),
				elemNoteBook.GetAttribute(L"nickname").c_str(),
				StrToCTime(elemNoteBook.GetAttribute(L"lastModifiedTime").c_str())
			};

			auto& oneNoteSections = oneNoteBook.mSections;

			// 処理待ちスタック
			std::vector<XmlElement> stkSections;
			stkSections.push_back(elemNoteBook);

			// 処理待ちスタックが空になるまで処理を続ける
			while(stkSections.empty() == false) {

				// スタックから要素を取り出す
				auto elemParent = stkSections.back();
				stkSections.pop_back();

				// 取り出した要素の子要素をforeachする
				XmlNodeList children = elemParent.ChildNodes();
				for (uint32_t j = 0; j < children.Size(); ++j) {
					XmlElement elemCurrent = children.GetAt(j).as<XmlElement>();
					tagName = elemCurrent.TagName();

					if (tagName == L"one:Section") {
						// セクションだったら、中身を解析してページ情報を得る
						ParseSectionElement(elemCurrent, oneNoteSections);
						continue;
					}
					else if (tagName == L"one:SectionGroup") {
						// セクショングループだった場合は処理待ちスタックに積む
						stkSections.push_back(elemCurrent);
						continue;
					}
				}
			}

			tmpBooks.push_back(oneNoteBook);
		}

		oneNoteBooks.swap(tmpBooks);

		return true;
	}

	bool ParseSectionElement(XmlElement& elemSection, std::vector<OneNoteSection>& oneNoteSections) {

		OneNoteSection oneNoteSection {
			elemSection.GetAttribute(L"ID").c_str(),
			elemSection.GetAttribute(L"name").c_str(),
			StrToCTime(elemSection.GetAttribute(L"lastModifiedTime").c_str())
		};

		XmlNodeList pages = elemSection.ChildNodes();
		for (uint32_t k = 0; k < pages.Size(); ++k) {
			XmlElement elemPage = pages.GetAt(k).as<XmlElement>();

			if (elemPage.TagName() != L"one:Page") {
				continue;
			}

			OneNotePage oneNotePage { 
				elemPage.GetAttribute(L"ID").c_str(),
					elemPage.GetAttribute(L"name").c_str(),
					StrToCTime(elemPage.GetAttribute(L"dateTime").c_str()),
					StrToCTime(elemPage.GetAttribute(L"lastModifiedTime").c_str()),
					elemPage.GetAttribute(L"isSubPage") == L"true"
			};

			oneNoteSection.mPages.push_back(oneNotePage);
		}

		oneNoteSections.push_back(oneNoteSection);
		return true;
	}



	CTime StrToCTime(const wchar_t* str) {
		int year, month, day, hour, minute, second;
		swscanf_s(str, L"%4d-%2d-%2dT%2d:%2d:%2d", &year, &month, &day, &hour, &minute, &second);

		CTime utcTime(year, month, day, hour, minute, second);
		return CTime(utcTime.GetTime());
	}

	CComPtr<IDispatch> mApp;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


OneNoteAppProxy::OneNoteAppProxy() : in(new PImpl)
{
}

OneNoteAppProxy::~OneNoteAppProxy()
{
}

bool OneNoteAppProxy::IsAvailable()
{
	// OneNoteのCLSIDを得る
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"OneNote.Application.12", &clsid);

	bool isInstalled = SUCCEEDED(hr);
	spdlog::info(_T("OneNote isAvailable:{}"), isInstalled);

	return isInstalled;
}

bool OneNoteAppProxy::GetHierarchy(std::vector<OneNoteBook>& books)
{
	// mAppが未初期化なら初期化する
	if (in->InitializeApp() == false) {
		return false;
	}

	// OneNote.Application.GetHierarchyをよぶ
	std::wstring xml;
	if (in->CallGetHierarchy(xml) == false) {
		return false;
	}

	// 得られたXMLを解析し、sectionsのリストを生成する
	return in->ParseHierarchyXML(xml, books);
}

bool OneNoteAppProxy::NavigateTo(LPCWSTR id)
{
	// mAppが未初期化なら初期化する
	if (in->InitializeApp() == false) {
		return false;
	}

	VARIANT result;
	VariantInit(&result);

	VARIANT argEmptyStr;
	VariantInit(&argEmptyStr);
	argEmptyStr.vt = VT_BSTR;
	CComBSTR argEmptyVal(L"");
	argEmptyStr.bstrVal = argEmptyVal;

	VARIANT argPage;
	VariantInit(&argPage);
	argPage.vt = VT_BSTR;
	CComBSTR argPageVal(id);
	argPage.bstrVal = argPageVal;

	HRESULT hr = AutoWrap(DISPATCH_METHOD, &result, in->mApp, L"NavigateTo", 2, &argEmptyStr, &argPage);
	if (FAILED(hr)) {
		spdlog::error("OneNoteAppProxy::NavigateTo failed hr={}", hr);
	}

	return SUCCEEDED(hr);
}



}}} // end of namespace launcherapp::commands::onenote

