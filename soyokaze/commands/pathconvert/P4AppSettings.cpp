#include "pch.h"
#include "P4AppSettings.h"
#include <atlbase.h>

#import "msxml6.dll" exclude("ISequentialStream","_FILETIME")named_guids

namespace launcherapp {
namespace commands {
namespace pathconvert {


struct P4AppSettings::PImpl
{
	CString GetSettingsFilePath();
	bool FindRecentConnectionsNode(CComPtr<IXMLDOMElement>& domElem, CComPtr<IXMLDOMNode>& out);
	bool FindRecentConnectionsNode(CComPtr<IXMLDOMNode>& domElem, CComPtr<IXMLDOMNode>& out);
	bool IsRecentConnectionNode(CComPtr<IXMLDOMNode>& node);

	bool ParseConnectionString(const CString& str);

	std::vector<ITEM> mItems;
};

CString P4AppSettings::PImpl::GetSettingsFilePath()
{
	CString path;
	LPTSTR p = path.GetBuffer(MAX_PATH_NTFS);

	size_t reqLen = 0;
	_tgetenv_s(&reqLen, p, MAX_PATH_NTFS, _T("USERPROFILE"));
	PathAppend(p, _T(".p4qt/ApplicationSettings.xml"));
	path.ReleaseBuffer();

	return path;
}

// <StringList varName="RecentConnections">ノードを探す
bool P4AppSettings::PImpl::FindRecentConnectionsNode(CComPtr<IXMLDOMElement>& domElem, CComPtr<IXMLDOMNode>& out)
{
	CComPtr<IXMLDOMNodeList> childList;
	domElem->get_childNodes(&childList);

	long childrenCount = 0;
	HRESULT hr = childList->get_length(&childrenCount);

	for (long i = 0; i < childrenCount; ++i) {

		CComPtr<IXMLDOMNode> item;
		hr = childList->get_item(i, &item);

		if (IsRecentConnectionNode(item)) {
			out = item;
			return true;
		}

		if (FindRecentConnectionsNode(item, out)) {
			return true;
		}
	}
	return false;
}

bool P4AppSettings::PImpl::FindRecentConnectionsNode(CComPtr<IXMLDOMNode>& node, CComPtr<IXMLDOMNode>& out)
{
	CComPtr<IXMLDOMNodeList> childList;
	node->get_childNodes(&childList);

	long childrenCount = 0;
	HRESULT hr = childList->get_length(&childrenCount);

	for (long i = 0; i < childrenCount; ++i) {

		CComPtr<IXMLDOMNode> item;
		hr = childList->get_item(i, &item);

		if (IsRecentConnectionNode(item)) {
			out = item;
			return true;
		}

		if (FindRecentConnectionsNode(item, out)) {
			return true;
		}
	}
	return false;
}

bool P4AppSettings::PImpl::IsRecentConnectionNode(CComPtr<IXMLDOMNode>& node)
{
	CComBSTR nodeName;
	HRESULT hr = node->get_baseName(&nodeName);
	if (FAILED(hr)) {
		return false;
	}

	if (nodeName != _T("StringList")) {
		return false;
	}

	CComPtr<IXMLDOMNamedNodeMap> attrMap;
	hr = node->get_attributes(&attrMap);
	if (FAILED(hr)) {
		return false;
	}

	CComPtr<IXMLDOMNode> attrValueItem;
	hr = attrMap->getNamedItem(CComBSTR(_T("varName")), &attrValueItem);
	if (FAILED(hr)) {
		return false;
	}

	// 属性の値を得る(<node attr="...">の...部分)
	CComBSTR attrValueStr;
	hr = attrValueItem->get_text(&attrValueStr);
	if (FAILED(hr)) {
		return false;
	}

	return attrValueStr == _T("RecentConnections");
}

bool P4AppSettings::PImpl::ParseConnectionString(const CString& str)
{
	int n = 0;

	ITEM item;

	item.mPort = str.Tokenize(_T(","), n);
	if (item.mPort.IsEmpty()) {
		return false;
	}

	item.mUser = str.Tokenize(_T(","), n);
	if (item.mUser.IsEmpty()) {
		return false;
	}

	item.mClient = str.Tokenize(_T(","), n);
	if (item.mClient.IsEmpty()) {
		return false;
	}

	mItems.push_back(item);

	return true;
}


P4AppSettings::P4AppSettings() : in(new PImpl)
{
	HRESULT hr = CoInitialize(nullptr);
	if (FAILED(hr)) {
		SPDLOG_ERROR(_T("Failed to CoInitialize!"));
	}

	CString filePath = in->GetSettingsFilePath();
	if (PathFileExists(filePath) == FALSE) {
		// p4がインストールされている環境でない
		return;
	}

	try {
		CComPtr<IXMLDOMDocument> pXmlDom;
		hr = pXmlDom.CoCreateInstance(CLSID_DOMDocument);
		if (FAILED(hr)) {
			SPDLOG_ERROR(_T("Failed to CoCreateInstance of DOMDocument!"));
			return;
		}

		CComBSTR argVal(filePath);

		VARIANT arg1;
		VariantInit(&arg1);
		arg1.vt = VT_BSTR;
		arg1.bstrVal = argVal;

		VARIANT_BOOL arg2;
		hr = pXmlDom->load(arg1, &arg2);
		if (FAILED(hr)) {
			return;
		}

		CComPtr<IXMLDOMElement> domElem;
		hr = pXmlDom->get_documentElement(&domElem);

		// <StringList varName="RecentConnections">ノードを探す
		CComPtr<IXMLDOMNode> recentConnectionsNode;
		if (in->FindRecentConnectionsNode(domElem, recentConnectionsNode) == false) {
			return;
		}

		// RecentConnectionsの子ノードにある各種要素を取得する
		CComPtr<IXMLDOMNodeList> stringList;
		recentConnectionsNode->get_childNodes(&stringList);

		long stringCount;
		hr = stringList->get_length(&stringCount);
		for (long i = 0; i < stringCount; ++i) {

			CComPtr<IXMLDOMNode> stringItem;
			hr = stringList->get_item(i, &stringItem);

			CComBSTR nodeName;
			hr = stringItem->get_baseName(&nodeName);
			if (FAILED(hr)) {
				continue;
			}
			if (nodeName != _T("String")) {
				continue;
			}
			// ノードのテキストを得る(<node>...</node>の...部分)
			CComBSTR value;
			stringItem->get_text(&value);

			in->ParseConnectionString((LPCTSTR)value);
		}
	}
	catch(...) {
	}
}

P4AppSettings::~P4AppSettings()
{
	CoUninitialize();
}

size_t P4AppSettings::GetCount()
{
	return in->mItems.size();
}

void P4AppSettings::GetItems(std::vector<ITEM>& items)
{
	items = in->mItems;
}

} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp


