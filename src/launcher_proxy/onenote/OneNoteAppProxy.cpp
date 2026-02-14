#include "OneNoteAppProxy.h"

#include <windows.h>
#include "ScopeAttachThreadInput.h"
#include "StringUtil.h"
#include "AutoWrap.h"
#include <oleauto.h>
#include <atlbase.h>
#include <spdlog/spdlog.h>
#include <thread>

#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.Foundation.Collections.h>

using json = nlohmann::json;

using namespace winrt;
using namespace winrt::Windows::Data::Xml::Dom;

namespace launcherproxy { 

struct OneNoteAppProxy::PImpl
{
	void RunWatchThread();

	bool InitializeApp(CComPtr<IDispatch>& app);
	bool CallGetHierarchy(CComPtr<IDispatch>& oneNoteApp, std::wstring& hierarchyXml);
	bool ParseHierarchyXML(const std::wstring& xml, json& oneNoteBooks);
	bool ParseSectionElement(XmlElement& elemSection, json& oneNoteSections);

	void SetErrorMessage(const char* msg) {
		std::lock_guard<std::mutex> lock(mMutex);
		mErrMsg = msg;
	}


	HANDLE mRequestEvent{nullptr};
	HANDLE mAbortEvent{nullptr};
	HANDLE mAbortedEvent{nullptr};
	std::mutex mMutex;
	std::string mErrMsg;
	bool mHasContent{false};
	json mNoteBooks;
};

void OneNoteAppProxy::PImpl::RunWatchThread()
{
	mRequestEvent = CreateEvent(nullptr, TRUE, TRUE, nullptr);
	mAbortEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	mAbortedEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	std::thread th([&]() {

		HRESULT hr = CoInitialize(NULL);

		CComPtr<IDispatch> app;
		if (InitializeApp(app) == false) {
			SetErrorMessage("Failed to Initialize OneNote App.");
			SetEvent(mAbortedEvent);
			return;
		}

		while(WaitForSingleObject(mAbortEvent, 0) == WAIT_TIMEOUT) {

			if (WaitForSingleObject(mRequestEvent, 150) != WAIT_OBJECT_0) {
				continue;
			}
			ResetEvent(mRequestEvent);

			// OneNote.Application.GetHierarchyをよぶ
			std::wstring xml;
			if (CallGetHierarchy(app, xml) == false) {
				SetErrorMessage("Failed to GetHierarchy.");
				continue;
			}

			// 得られたXMLを解析し、sectionsのリストを生成する
			json books = {};
			if (ParseHierarchyXML(xml, books) == false) {
				SetErrorMessage("Failed to parse xml.");
				continue;
			}

			// 結果をセットする
			SetErrorMessage("");
			std::lock_guard<std::mutex> lock(mMutex);
			mNoteBooks = books;
			mHasContent = true;
		}

		SetEvent(mAbortedEvent);

		CoUninitialize();
	});
	th.detach();
}

bool OneNoteAppProxy::PImpl::InitializeApp(CComPtr<IDispatch>& app) {

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
bool OneNoteAppProxy::PImpl::CallGetHierarchy(CComPtr<IDispatch>& oneNoteApp, std::wstring& hierarchyXml)
{

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

bool OneNoteAppProxy::PImpl::ParseHierarchyXML(const std::wstring& xml, json& oneNoteBooks)
{
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

bool OneNoteAppProxy::PImpl::ParseSectionElement(XmlElement& elemSection, json& oneNoteSections)
{

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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



OneNoteAppProxy::OneNoteAppProxy() : in(new PImpl)
{
	in->RunWatchThread();
}

OneNoteAppProxy::~OneNoteAppProxy()
{
	CloseHandle(in->mRequestEvent);
	in->mRequestEvent = nullptr;
	CloseHandle(in->mAbortEvent);
	in->mAbortEvent = nullptr;
	CloseHandle(in->mAbortedEvent);
	in->mAbortedEvent = nullptr;
}

OneNoteAppProxy* OneNoteAppProxy::GetInstance()
{
	static OneNoteAppProxy inst;
	return &inst;
}

void OneNoteAppProxy::Abort()
{
	SetEvent(in->mAbortEvent);
	WaitForSingleObject(in->mAbortedEvent, 3000);
}

// ブック一覧を取得する
bool OneNoteAppProxy::GetHierarchy(nlohmann::json& noteBooks)
{
	std::lock_guard<std::mutex> lock(in->mMutex);

	SetEvent(in->mRequestEvent);

	if (in->mHasContent == false) {
		return false;
	}
	if (in->mErrMsg.empty() == false) {
		return false;
	}

	noteBooks = in->mNoteBooks;
	return true;
}

std::string OneNoteAppProxy::GetErrorMessage()
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	return in->mErrMsg;
}

} // end of namespace 

