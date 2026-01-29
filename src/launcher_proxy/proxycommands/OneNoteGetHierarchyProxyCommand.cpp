#include "OneNoteGetHierarchyProxyCommand.h"
#include <windows.h>
#include "ScopeAttachThreadInput.h"
#include "StringUtil.h"
#include "AutoWrap.h"
#include <oleauto.h>
#include <atlbase.h>
#include <spdlog/spdlog.h>

#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.Foundation.Collections.h>

using namespace winrt;
using namespace winrt::Windows::Data::Xml::Dom;

using json = nlohmann::json;

namespace launcherproxy { 


struct OneNoteGetHierarchyProxyCommand::PImpl
{
	bool InitializeApp(CComPtr<IDispatch>& app) {

		CLSID CLSID_OneNote;
		HRESULT hr = CLSIDFromProgID(L"OneNote.Application.12", &CLSID_OneNote);
		if (FAILED(hr)) {
			return false;
		}
		hr = CoCreateInstance(CLSID_OneNote, nullptr, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void**)&app);
		if (FAILED(hr)) {
			return false;
		}
		return true;
	}

	// OneNote.Application.GetHierarchyをよぶ
	bool CallGetHierarchy(CComPtr<IDispatch>& oneNoteApp, std::wstring& hierarchyXml) {

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

	bool ParseHierarchyXML(const std::wstring& xml, json& oneNoteBooks) {

		XmlDocument doc;

		try {
			doc.LoadXml(xml.c_str());
		} catch(const winrt::hresult_error& e) {
			auto msg = e.message();
			//spdlog::error(L"Failed to parse XML code:{0} message:{1}", static_cast<uint32_t>(e.code()), msg.c_str());
			return false;
		}

		std::string dst;

		XmlElement elemNoteBooks = doc.DocumentElement();  // NoteBooks

		json tmpBooks = json::array();

		XmlNodeList noteBooks = elemNoteBooks.ChildNodes();
		for (uint32_t i = 0; i < noteBooks.Size(); ++i) {
			XmlElement elemNoteBook = noteBooks.GetAt(i).as<XmlElement>();
			auto tagName = elemNoteBook.TagName();
			if (tagName != L"one:Notebook") {
				continue;
			}

			json oneNoteBook = {};
			oneNoteBook["ID"] = utf2utf(elemNoteBook.GetAttribute(L"ID").c_str(), dst);
			oneNoteBook["name"] = utf2utf(elemNoteBook.GetAttribute(L"name").c_str(), dst);
			oneNoteBook["nickname"] = utf2utf(elemNoteBook.GetAttribute(L"nickname").c_str(), dst);
			oneNoteBook["lastModifiedTime"] = utf2utf(elemNoteBook.GetAttribute(L"lastModifiedTime").c_str(), dst);

			json oneNoteSections = json::array();

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

			oneNoteBook["sections"] = oneNoteSections;

			tmpBooks.push_back(oneNoteBook);
		}

		oneNoteBooks.swap(tmpBooks);

		return true;
	}

	bool ParseSectionElement(XmlElement& elemSection, json& oneNoteSections) {

		std::string dst;

		json oneNoteSection = {};
		oneNoteSection["ID"] = utf2utf(elemSection.GetAttribute(L"ID").c_str(), dst);
		oneNoteSection["name"] = utf2utf(elemSection.GetAttribute(L"name").c_str(), dst);
		oneNoteSection["lastModifiedTime"] = utf2utf(elemSection.GetAttribute(L"lastModifiedTime").c_str(), dst);

		json oneNotePages = json::array();

		XmlNodeList pages = elemSection.ChildNodes();
		for (uint32_t k = 0; k < pages.Size(); ++k) {
			XmlElement elemPage = pages.GetAt(k).as<XmlElement>();

			if (elemPage.TagName() != L"one:Page") {
				continue;
			}

			json oneNotePage = {};
			oneNotePage["ID"] = utf2utf(elemPage.GetAttribute(L"ID").c_str(), dst);
			oneNotePage["name"] = utf2utf(elemPage.GetAttribute(L"name").c_str(), dst);
			oneNotePage["dateTime"] = utf2utf(elemPage.GetAttribute(L"dateTime").c_str(), dst);
			oneNotePage["lastModifiedTime"] = utf2utf(elemPage.GetAttribute(L"lastModifiedTime").c_str(), dst);
			oneNotePage["isSubPage"] = (elemPage.GetAttribute(L"isSubPage") == L"true");

			oneNotePages.push_back(oneNotePage);
		}

		oneNoteSection["pages"] = oneNotePages;
		oneNoteSections.push_back(oneNoteSection);
		return true;
	}
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



REGISTER_PROXYCOMMAND(OneNoteGetHierarchyProxyCommand)

OneNoteGetHierarchyProxyCommand::OneNoteGetHierarchyProxyCommand() : in(new PImpl)
{
}

OneNoteGetHierarchyProxyCommand::~OneNoteGetHierarchyProxyCommand()
{
}

std::string OneNoteGetHierarchyProxyCommand::GetName()
{
	return "onenote_gethierarchy";
}

bool OneNoteGetHierarchyProxyCommand::Execute(json& json_req, json& json_res)
{
	CComPtr<IDispatch> app;
	if (in->InitializeApp(app) == false) {
		json_res["reason"] = "Failed to Initialize OneNote App.";
		return false;
	}

	// OneNote.Application.GetHierarchyをよぶ
	std::wstring xml;
	if (in->CallGetHierarchy(app, xml) == false) {
		json_res["reason"] = "Failed to GetHierarchy.";
		return false;
	}

	// 得られたXMLを解析し、sectionsのリストを生成する
	json books = {};
	if (in->ParseHierarchyXML(xml, books) == false) {
		json_res["reason"] = "Failed to parse xml.";
		return false;
	}

	json_res["result"] = true;
	json_res["books"] = books;

	return true;
}

} //"sheet does not found."end of namespace 



