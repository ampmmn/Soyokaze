#include "pch.h"
#include "MMCSnapins.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "utility/Path.h"
#include "utility/ManualEvent.h"
#include "utility/ScopedResetEvent.h"
#include <thread>

#import "msxml6.dll" exclude("ISequentialStream","_FILETIME")named_guids

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace mmc {

struct MMCSnapins::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void RunCollectTask();
	void EnumItems(std::vector<MMCSnapin>& items);
	bool LoadItem(LPCTSTR filePath, MMCSnapin& item);

	void OnAppFirstBoot() override
 	{
		OnAppNormalBoot();
	}
	void OnAppNormalBoot() override
 	{
		mWaitEvt.Set();
		RunCollectTask();
	}
	void OnAppPreferenceUpdated() override {}
	void OnAppExit() override {}

	std::mutex mMutex;
	std::vector<MMCSnapin> mItems;
	ManualEvent mWaitEvt;
};

void MMCSnapins::PImpl::RunCollectTask()
{
	std::thread th([&]() {

			ScopedResetEvent scoped_event(mWaitEvt, true);

			Sleep(500);
			HRESULT hr = CoInitialize(NULL);
			if (FAILED(hr)) {
				spdlog::error(_T("Failed to CoInitialize!"));
			}

			std::vector<MMCSnapin> items;
			EnumItems(items);

			std::lock_guard<std::mutex> lock(mMutex);
			mItems.swap(items);

			CoUninitialize();
			});
	th.detach();
}

void MMCSnapins::PImpl::EnumItems(std::vector<MMCSnapin>& items)
{
	spdlog::info(_T("start Enum MMC snapins."));

	Path pattern(Path::SYSTEMDIR);
	pattern.Append(_T("*.msc"));

	std::vector<MMCSnapin> tmpItems;

	CFileFind f;
	BOOL isLoop = f.FindFile(pattern, 0);
	while (isLoop) {
		isLoop = f.FindNextFile();
		if (f.IsDots() || f.IsDirectory()) {
			continue;
		}

		CString filePath = f.GetFilePath();

		// 言語別のファイルパスに置き換える
		Path langFilePath;
		ULONG maxFilePath = (ULONG)langFilePath.size();
		ULONG maxLangLen = 256;
		ULONGLONG enumId = 0;
		BOOL isLangFileExists = GetFileMUIPath(MUI_USER_PREFERRED_UI_LANGUAGES, filePath, 0, &maxLangLen, langFilePath, &maxFilePath, &enumId);

		LPCTSTR path = isLangFileExists ? (LPCTSTR)langFilePath : (LPCTSTR)filePath;

		MMCSnapin item;
		if (LoadItem(path, item) == false) {
			continue;
		}
		tmpItems.push_back(item);
	}
	f.Close();

	items.swap(tmpItems);
	spdlog::info(_T("end Enum MMC snapins."));
}

static bool FindVirtualAttributesNode(CComPtr<IXMLDOMElement>& domElem, CComPtr<IXMLDOMNode>& output)
{
	CComPtr<IXMLDOMNodeList> childList;
	domElem->get_childNodes(&childList);

	long count = 0;
	HRESULT hr = childList->get_length(&count);

	for (long i = 0; i < count; ++i) {

		CComPtr<IXMLDOMNode> item;
		hr = childList->get_item(i, &item);

		CComBSTR nodeName;
		hr = item->get_baseName(&nodeName);
		if (FAILED(hr)) {
			continue;
		}

		if (nodeName != _T("VisualAttributes")) {
			continue;
		}

		output = item;
		return true;
	}
	return false;
}

static bool FindApplicationTitleId(CComPtr<IXMLDOMNode>& node, CString& titleId)
{
	CComPtr<IXMLDOMNodeList> childList;
	node->get_childNodes(&childList);

	long count = 0;
	HRESULT hr = childList->get_length(&count);

	for (long i = 0; i < count; ++i) {

		CComPtr<IXMLDOMNode> item;
		hr = childList->get_item(i, &item);

		CComBSTR nodeName;
		hr = item->get_baseName(&nodeName);
		if (FAILED(hr)) {
			continue;
		}

		if (nodeName != _T("String")) {
			continue;
		}

		CComPtr<IXMLDOMNamedNodeMap> attrMap;
		hr = item->get_attributes(&attrMap);
		if (FAILED(hr)) {
			continue;
		}
//      <String Name="ApplicationTitle" ID="2"/>

		CComPtr<IXMLDOMNode> attrValueItem;
		hr = attrMap->getNamedItem(CComBSTR(_T("Name")), &attrValueItem);
		if (FAILED(hr)) {
			continue;
		}

		// 属性の値を得る(<node attr="...">の...部分)
		CComBSTR attrValueStr;
		hr = attrValueItem->get_text(&attrValueStr);
		if (FAILED(hr)) {
			continue;
		}
		if (attrValueStr != _T("ApplicationTitle")) {
			continue;
		}

		CComPtr<IXMLDOMNode> attrValueId;
		hr = attrMap->getNamedItem(CComBSTR(_T("ID")), &attrValueId);
		if (FAILED(hr)) {
			continue;
		}

		CComBSTR attrValueStrId;
		hr = attrValueId->get_text(&attrValueStrId);
		if (FAILED(hr)) {
			continue;
		}
		titleId = attrValueStrId;
		return true;
	}
	return false;
}

static void ResolveEnv(CString& str)
{
	for(;;) {
		int n = str.Find(_T('%'), 0);
		if (n == -1) {
			return;
		}

		int n2 = str.Find(_T('%'), n+1);
		if (n2 == -1) {
			return;
		}

		CString part = str.Mid(n, n2-n+1);

		CString newValue;
		CString valName = part.Mid(1,part.GetLength()-2);
		size_t reqLen = 0;
		if (_tgetenv_s(&reqLen, nullptr, 0, valName) == 0) {
			_tgetenv_s(&reqLen, newValue.GetBuffer((int)reqLen), reqLen, valName);
			newValue.ReleaseBuffer();
		}
		str.Replace(part, newValue);
	}
}

static bool FindIconFilePath(CComPtr<IXMLDOMNode>& node, CString& path, CString& index)
{
	CComPtr<IXMLDOMNodeList> childList;
	node->get_childNodes(&childList);

	long count = 0;
	HRESULT hr = childList->get_length(&count);

	for (long i = 0; i < count; ++i) {

		CComPtr<IXMLDOMNode> item;
		hr = childList->get_item(i, &item);

		CComBSTR nodeName;
		hr = item->get_baseName(&nodeName);
		if (FAILED(hr)) {
			continue;
		}

//      <Icon Index="4" File="%windir%\system32\devmgr.dll">
		if (nodeName != _T("Icon")) {
			continue;
		}

		CComPtr<IXMLDOMNamedNodeMap> attrMap;
		hr = item->get_attributes(&attrMap);
		if (FAILED(hr)) {
			continue;
		}

		CComPtr<IXMLDOMNode> attrValueItem;
		hr = attrMap->getNamedItem(CComBSTR(_T("Index")), &attrValueItem);
		if (FAILED(hr)) {
			continue;
		}

		// 属性の値を得る(<node attr="...">の...部分)
		CComBSTR attrValueStr;
		hr = attrValueItem->get_text(&attrValueStr);
		if (FAILED(hr)) {
			continue;
		}

		index = attrValueStr;

		attrValueItem.Release();
		hr = attrMap->getNamedItem(CComBSTR(_T("File")), &attrValueItem);
		if (FAILED(hr)) {
			continue;
		}
		hr = attrValueItem->get_text(&attrValueStr);
		if (FAILED(hr)) {
			continue;
		}

		path = attrValueStr;
		// 環境変数の解決をする
		ResolveEnv(path);

		// eventvwr.mscのIconタグのFile属性値が"%SystemRoot%\Windows"になっている対策
		path.Replace(_T("WINDOWS\\Windows\\system32"), _T("Windows\\system32"));

		return true;
	}
	return false;
}

static bool FindStringTablesNode(CComPtr<IXMLDOMElement>& domElem, CComPtr<IXMLDOMNode>& output)
{
	CComPtr<IXMLDOMNodeList> childList;
	domElem->get_childNodes(&childList);

	long count = 0;
	HRESULT hr = childList->get_length(&count);

	for (long i = 0; i < count; ++i) {

		CComPtr<IXMLDOMNode> item;
		hr = childList->get_item(i, &item);

		CComBSTR nodeName;
		hr = item->get_baseName(&nodeName);
		if (FAILED(hr)) {
			continue;
		}

		if (nodeName != _T("StringTables")) {
			continue;
		}

		output = item;
		return true;
	}
	return false;
}

static bool FindStringFromTable(
	CComPtr<IXMLDOMNode>& stringTablesNode,
	const CString& titleId,
	CString& titleString
)
{
//    <StringTables>
//      <StringTable>
//        <Strings>
//          <String ID="2" Refs="2">ディスクの管理</String>

	CComPtr<IXMLDOMNodeList> childList;
	stringTablesNode->get_childNodes(&childList);

	long count = 0;
	HRESULT hr = childList->get_length(&count);

	for (long i = 0; i < count; ++i) {

		CComPtr<IXMLDOMNode> stringTableItem;
		hr = childList->get_item(i, &stringTableItem);

		CComBSTR nodeName;
		hr = stringTableItem->get_baseName(&nodeName);
		if (FAILED(hr)) {
			continue;
		}

		if (nodeName != _T("StringTable")) {
			continue;
		}

		CComPtr<IXMLDOMNodeList> childList2;
		stringTableItem->get_childNodes(&childList2);

		long count2 = 0;
		hr = childList2->get_length(&count2);

		for (long j = 0; j < count2; ++j) {

			CComPtr<IXMLDOMNode> stringsNode;
			hr = childList2->get_item(j, &stringsNode);

			hr = stringsNode->get_baseName(&nodeName);
			if (FAILED(hr)) {
				continue;
			}
			if (nodeName != _T("Strings")) {
				continue;
			}

			CComPtr<IXMLDOMNodeList> childList3;
			stringsNode->get_childNodes(&childList3);

			long countOfString = 0;
			hr = childList3->get_length(&countOfString);
			for (long k = 0; k < countOfString; ++k) {

				CComPtr<IXMLDOMNode> stringNode;
				hr = childList3->get_item(k, &stringNode);

				hr = stringNode->get_baseName(&nodeName);
				if (FAILED(hr)) {
					continue;
				}
				if (nodeName != _T("String")) {
					continue;
				}

				CComPtr<IXMLDOMNamedNodeMap> attrMap;
				hr = stringNode->get_attributes(&attrMap);
				if (FAILED(hr)) {
					continue;
				}
				CComPtr<IXMLDOMNode> attrValueItem;
				hr = attrMap->getNamedItem(CComBSTR(_T("ID")), &attrValueItem);
				if (FAILED(hr)) {
					continue;
				}
				// 属性の値を得る(<node attr="...">の...部分)
				CComBSTR attrValueStr;
				hr = attrValueItem->get_text(&attrValueStr);
				if (FAILED(hr)) {
					return false;
				}

				if (attrValueStr != (LPCTSTR)titleId) {
					continue;
				}

				// ノードのテキストを得る(<node>...</node>の...部分)
				CComBSTR value;
				stringNode->get_text(&value);

				titleString = (LPCTSTR)value;
				return true;
			}
		}
	}
	return false;
}



//<MMC_ConsoleFile ConsoleVersion="3.0" ProgramMode="UserSDI">
//    <VisualAttributes>
//      <String Name="ApplicationTitle" ID="2"/>
//      <Icon Index="4" File="%windir%\system32\devmgr.dll">
//        <Image Name="Large" BinaryRefIndex="0"/>
//    <StringTables>
//      <StringTable>
//        <Strings>
//          <String ID="2" Refs="2">ディスクの管理</String>
bool MMCSnapins::PImpl::LoadItem(
	LPCTSTR filePath,
 	MMCSnapin& item
)
{
	spdlog::debug(_T("loading {}"), filePath);

	try {
		CComPtr<IXMLDOMDocument> pXmlDom;
		HRESULT hr = pXmlDom.CoCreateInstance(CLSID_DOMDocument);
		if (FAILED(hr)) {
			spdlog::error(_T("Failed to createinstance of DOMDocument!"));
			return false;
		}

		CComBSTR argVal(filePath);

		VARIANT arg1;
		VariantInit(&arg1);
		arg1.vt = VT_BSTR;
		arg1.bstrVal = argVal;

		VARIANT_BOOL arg2;
		hr = pXmlDom->load(arg1, &arg2);
		if (FAILED(hr)) {
			return false;
		}

		CComPtr<IXMLDOMElement> domElem;
		hr = pXmlDom->get_documentElement(&domElem);

		CComPtr<IXMLDOMNode> virtualAttrNode;
		if (FindVirtualAttributesNode(domElem, virtualAttrNode) == false) {
			spdlog::warn(_T("VirtualAttributes node is missing."));
			return false;
		}

		// タイトル文字列の取得(取得できない場合はファイル名で代替する)
		CString titleString(PathFindFileName(filePath)); 
		CString titleId;
		if (FindApplicationTitleId(virtualAttrNode, titleId)) {
			CComPtr<IXMLDOMNode> stringTablesNode;
			if (FindStringTablesNode(domElem, stringTablesNode)) {
				if (FindStringFromTable(stringTablesNode, titleId, titleString) == false) {
					spdlog::warn(_T("Failed to reolve TitleString."));
				}
			}
			else {
				spdlog::warn(_T("StringTables is missing."));
			}
		}
		else {
			spdlog::warn(_T("ApplicationTitle is missing."));
		}

		CString iconPath;
		CString iconIndex;
		if (FindIconFilePath(virtualAttrNode, iconPath, iconIndex) == false) {
			spdlog::warn(_T("Icon file is missing."));
		}

		item.mDisplayName = titleString;
		item.mMscFilePath = filePath;
		item.mIconFilePath = iconPath;
		item.mIconIndex = _ttoi(iconIndex);

		return true;
	}
	catch(...) {
		spdlog::warn(_T("an exception occurred."));
		return false;
	}
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



MMCSnapins::MMCSnapins() : in(new PImpl)
{
	in->mWaitEvt.Set();
}

MMCSnapins::~MMCSnapins()
{
	// 更新をするタスクの完了を待つ
	in->mWaitEvt.WaitFor(5000);
}

void MMCSnapins::GetSnapins(
		std::vector<MMCSnapin>& items
)
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	items = in->mItems;
}



} // end of namespace mmc
} // end of namespace commands
} // end of namespace launcherapp

