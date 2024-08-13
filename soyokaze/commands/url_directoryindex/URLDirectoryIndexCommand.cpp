#include "pch.h"
#include "framework.h"
#include "URLDirectoryIndexCommand.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommandParam.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommandEditDialog.h"
#include "commands/core/CommandRepository.h"
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include "commands/activate_window/AutoWrap.h"
#include "utility/CharConverter.h"

#include <deque>
#include <tidy.h>
#include <tidybuffio.h>
#include "spdlog/stopwatch.h"

#import "msxml6.dll" exclude("ISequentialStream","_FILETIME")named_guids

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

using IServerXMLHTTPRequestPtr = MSXML2::IServerXMLHTTPRequestPtr;

using LinkItems = std::vector<std::pair<CString, CString> >;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct URLDirectoryIndexCommand::PImpl
{
	PImpl()
	{
	}
	~PImpl()
	{
	}
	bool LoadCandidates();
	bool ExtractCandidates(const CString& text);

	void Query(Pattern* pattern, DirectoryIndexQueryResult& results);

	bool IsLoaded()
	{
		return mIsLoaded;
	}

	size_t GetCandidatesCount()
	{
		return mLinkItems.size();
	}

	CommandParam mParam;
	CommandHotKeyAttribute mHotKeyAttr;
	CString mErrMsg;

	LinkItems mLinkItems;

	CString mSubPath;

	bool mIsEmpty = false;
	bool mIsLoaded = false;
	bool mIsLoadOK = false;
};

// 候補一覧の生成を開始する
bool URLDirectoryIndexCommand::PImpl::LoadCandidates()
{
	spdlog::stopwatch sw;

	// 既存のExcel.Applicationインスタンスを取得する
	IServerXMLHTTPRequestPtr pIXMLHTTPRequest;
	HRESULT hr = pIXMLHTTPRequest.CreateInstance(L"MSXML2.ServerXMLHTTP.6.0");
	if(FAILED(hr)) {
		spdlog::debug(_T("Failed to createinstance MSXML2.ServerXMLHTTP.6.0"));
		return false;
	}

	spdlog::debug("createinstance {:.6f} s.", sw);
	// ToDo:認証が必要な場合は設定する

	// ToDo:プロキシを経由する場合は設定する

	// サブパスを連結
	CString url = mParam.CombineURL(mSubPath);

	hr = pIXMLHTTPRequest->open(L"GET", (LPCWSTR)url, true);
	if(FAILED(hr)) {
		spdlog::debug(_T("Failed to open"));
		return false;
	}
	spdlog::debug("open {:.6f} s.", sw);

	hr = pIXMLHTTPRequest->send();  
	if(FAILED(hr)) {
		spdlog::debug(_T("Failed to send"));
		return false;
	}

	spdlog::debug("send {:.6f} s.", sw);

	pIXMLHTTPRequest->waitForResponse();

	if (pIXMLHTTPRequest->status >= 400) {
		spdlog::debug(_T("returned error"));
		return false;
	}

	spdlog::debug("waitforResponse {:.6f} s.", sw);

	CString text((LPCTSTR)pIXMLHTTPRequest->responseText);

	spdlog::debug(_T("URL : {}"), (LPCTSTR)url);
	spdlog::debug("download {:.6f} s.", sw);


	// 取得したテキストを解析
	return ExtractCandidates(text);
}

// HTMLからリンク一覧を生成する
bool URLDirectoryIndexCommand::PImpl::ExtractCandidates(
	const CString& text
)
{
	spdlog::stopwatch sw;
	launcherapp::utility::CharConverter conv;

 	TidyDoc doc = tidyCreate();
	tidyOptSetBool(doc, TidyForceOutput, no);
	tidyOptSetInt(doc, TidyWrapLen, 4096);

  //TidyBuffer tidy_errbuf = {0};
	//tidySetErrorBuffer(doc, &tidy_errbuf);

	CStringA srcA(text);
 
	int err = tidyParseString(doc, srcA);
	spdlog::debug(_T("tidyParseString returned {}."), err);
	if (err < 0) {
		mIsLoadOK = false;
		mIsLoaded = true;
		return false;
	}

	int doctype = tidyReportDoctype(doc);
	spdlog::debug(_T("tidyReportDoctype returned {}."), doctype);

	err = tidyCleanAndRepair(doc);
	err = tidyRunDiagnostics(doc);

	auto rootNode = tidyGetRoot(doc);

	if (rootNode == nullptr) {
		spdlog::debug(_T("tidyGetRoot returned nullptr."));
	}

	std::deque<TidyNode> stk;
	stk.push_back(rootNode);

	LinkItems links;

	while(stk.empty() == false) {
		auto curNode = stk.front();
		stk.pop_front();

		auto curName = tidyNodeGetName(curNode);


		for (TidyNode childNode = tidyGetChild(curNode); childNode != nullptr;
		     childNode = tidyGetNext(childNode)) {

			auto name = tidyNodeGetName(childNode);
			if(name) {				

				CString href;
				bool isA = stricmp(name, "a") == 0;

				for(auto attr = tidyAttrFirst(childNode); attr; attr = tidyAttrNext(attr) ) {
					auto attrName = tidyAttrName(attr);
					auto attrValue = tidyAttrValue(attr);
					if (isA && stricmp(attrName, "href") == 0) {
						conv.Convert(attrValue, href);
					}
				}
				stk.push_back(childNode);

				if (isA) {
					auto textNode = tidyGetChild(childNode);
					if (textNode) {
						TidyBuffer buf;
						tidyBufInit(&buf);
						tidyNodeGetText(doc, textNode, &buf);

						CString displayName;
						conv.Convert((const char*)buf.bp, displayName);
						if (isA) {
							displayName.Trim();
							links.push_back(std::pair<CString, CString>(href, displayName));
						}
						tidyBufFree(&buf);
					}
				}
				continue;
			}
			stk.push_back(childNode);
		}
	}
 
  tidyRelease(doc);

	mLinkItems.swap(links);
	mIsLoaded = true;
	mIsLoadOK = true;

	// 候補の抽出が完了したことを通知
	SharedHwnd sharedWnd;
	PostMessage(sharedWnd.GetHwnd(), WM_APP+15, 0, 0);

	spdlog::debug("HTML parse {:.6f} s.", sw);

	return true;
}


void URLDirectoryIndexCommand::PImpl::Query(Pattern* pattern, DirectoryIndexQueryResult& results)
{
	DirectoryIndexQueryResult tmpList;

	std::vector<CString> words;
	pattern->GetRawWords(words);
	if (words.size() == 1) {
		// 入力キーワードが空文字の場合は全てを候補に追加する
		for(auto& item : mLinkItems) {
			QueryResult result;
			result.mLinkPath = item.first;
			result.mLinkText = item.second;
			result.mURL = mParam.CombineURL(item.first);
			result.mMatchLevel = Pattern::PartialMatch;
			tmpList.push_back(result);
		}
		tmpList.swap(results);
		return;
	}


	for(auto& item : mLinkItems) {
		int level = pattern->Match(item.second, 1);
		if (level == Pattern::Mismatch) {
			continue;
		}

		QueryResult result;
		result.mLinkPath = item.first;
		result.mLinkText = item.second;
		result.mURL = mParam.CombineURL(item.first);
		result.mMatchLevel = level;

		tmpList.push_back(result);
	}

	tmpList.swap(results);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString URLDirectoryIndexCommand::GetType() { return _T("URLDirectoryIndex"); }

URLDirectoryIndexCommand::URLDirectoryIndexCommand() : in(std::make_unique<PImpl>())
{
}

URLDirectoryIndexCommand::~URLDirectoryIndexCommand()
{
}


void URLDirectoryIndexCommand::SetSubPath(const CString& subPath)
{
	if (subPath.Left(1) == _T('/')) {
		in->mSubPath = subPath;
	}
	else {
		if (in->mSubPath.IsEmpty() == FALSE && in->mSubPath.Right(1) != _T('/')) {
			in->mSubPath += _T('/');
		}
		in->mSubPath += subPath;
	}
	ClearCache();
}

void URLDirectoryIndexCommand::LoadCanidates()
{
	in->LoadCandidates();
}

void URLDirectoryIndexCommand::ClearCache()
{
	in->mLinkItems.clear();
	in->mIsEmpty = false;
	in->mIsLoaded = false;
	in->mIsLoadOK = false;
}

void URLDirectoryIndexCommand::Query(Pattern* pattern, DirectoryIndexQueryResult& results)
{
	// コマンド名が一致しなければ候補を表示しない
	if (GetName().CompareNoCase(pattern->GetFirstWord()) != 0) {
		ClearCache();
		in->mSubPath.Empty();
		spdlog::info(_T("clear state"));
		return;
	}
	in->Query(pattern, results);
}

CString URLDirectoryIndexCommand::GetName()
{
	return in->mParam.mName;
}


CString URLDirectoryIndexCommand::GetDescription()
{
	if (in->IsLoaded() == false) {
		return _T("(取得中)");
	}

	return in->mParam.mDescription;
}

CString URLDirectoryIndexCommand::GetGuideString()
{
	return _T("Enter:候補を表示する");
}

CString URLDirectoryIndexCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE(_T("DirectiryIndexコマンド"));
	return TEXT_TYPE;
}

BOOL URLDirectoryIndexCommand::Execute(const Parameter& param)
{
	UNREFERENCED_PARAMETER(param);

	if (in->mIsEmpty) {
		// 候補が存在しないとわかっている場合は何もしない
		return TRUE;
	}

	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 2, 1, 0);

	auto cmdline = GetName();
	cmdline += _T(" ");
	SendMessage(sharedWnd.GetHwnd(), WM_APP+11, 0, (LPARAM)(LPCTSTR)cmdline);
	return TRUE;
}

CString URLDirectoryIndexCommand::GetErrorString()
{
	return in->mErrMsg;
}

URLDirectoryIndexCommand& URLDirectoryIndexCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;
	return *this;
}

URLDirectoryIndexCommand& URLDirectoryIndexCommand::GetParam(CommandParam& param)
{
	param = in->mParam;
	return *this;
}


HICON URLDirectoryIndexCommand::GetIcon()
{
	return IconLoader::Get()->LoadPromptIcon();
}

int URLDirectoryIndexCommand::Match(Pattern* pattern)
{
	in->mIsEmpty = false;

	if (pattern->shouldWholeMatch() && pattern->Match(GetName()) == Pattern::WholeMatch) {
		// 内部のコマンド名マッチング用の判定
		return Pattern::WholeMatch;
	}
	else if (pattern->shouldWholeMatch() == false) {

		// 入力欄からの入力で、前方一致するときは候補に出す
		int level = pattern->Match(GetName());
		if (level == Pattern::FrontMatch) {
			return Pattern::FrontMatch;
		}
		if (level == Pattern::WholeMatch && pattern->GetWordCount() == 1) {

			if (in->IsLoaded()) {

				// 候補列挙済の場合、候補が存在する場合は、候補をFilterAdhocCommandとして表示するため、
				// このコマンド自身は表示しない
				if (in->GetCandidatesCount() > 0) {
					// 候補一覧生成済の場合は表示しない
					return Pattern::Mismatch;
				}

				// 候補件数が0の場合、その旨をURLDirectoryIndexCommand自身が表示する
				in->mIsEmpty = true;

				return Pattern::WholeMatch;
			}

			// このタイミングで候補一覧の生成を行う
			in->LoadCandidates();

			return Pattern::WholeMatch;
		}
	}

	// 通常はこちら
	return Pattern::Mismatch;
}

int URLDirectoryIndexCommand::EditDialog(HWND parent)
{
	URLDirectoryIndexCommandEditDialog dlg(CWnd::FromHandle(parent));
	dlg.SetOrgName(GetName());

	dlg.SetParam(in->mParam);
	dlg.mHotKeyAttr = in->mHotKeyAttr;

	if (dlg.DoModal() != IDOK) {
		spdlog::info(_T("Dialog cancelled."));
		return 1;
	}

	// 変更後の設定値で上書き
	dlg.GetParam(in->mParam);
	in->mHotKeyAttr = dlg.mHotKeyAttr;

	// 名前の変更を登録しなおす
	auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
	cmdRepo->ReregisterCommand(this);

	// 設定変更を反映するため、候補のキャッシュを消す
	ClearCache();

	return 0;
}

bool URLDirectoryIndexCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mHotKeyAttr;
	return true;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool URLDirectoryIndexCommand::IsPriorityRankEnabled()
{
	return true;
}

launcherapp::core::Command*
URLDirectoryIndexCommand::Clone()
{
	auto clonedObj = std::make_unique<URLDirectoryIndexCommand>();
	clonedObj->in->mParam = in->mParam;
	return clonedObj.release();
}

bool URLDirectoryIndexCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());

	entry->Set(_T("description"), GetDescription());

	entry->Set(_T("url"), in->mParam.mURL);

	return true;
}

bool URLDirectoryIndexCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != URLDirectoryIndexCommand::GetType()) {
		return false;
	}

	in->mParam.mName = entry->GetName();
	in->mParam.mDescription = entry->Get(_T("description"), _T(""));
	in->mParam.mURL = entry->Get(_T("url"), _T(""));

	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(GetName(), &in->mHotKeyAttr);

	return true;
}

bool URLDirectoryIndexCommand::NewDialog(const Parameter* param, URLDirectoryIndexCommand** newCmd)
{
	// 新規作成ダイアログを表示
	CString value;

	CommandParam tmpParam;

	if (param && param->GetNamedParam(_T("COMMAND"), &value)) {
		tmpParam.mName = value;
	}
	if (param && param->GetNamedParam(_T("PATH"), &value)) {
		tmpParam.mURL = value;
	}
	if (param && param->GetNamedParam(_T("DESCRIPTION"), &value)) {
		tmpParam.mDescription = value;
	}

	URLDirectoryIndexCommandEditDialog dlg;
	dlg.SetParam(tmpParam);

	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto cmd = std::make_unique<URLDirectoryIndexCommand>();

	if (newCmd) {
		*newCmd = cmd.get();
	}

	dlg.GetParam(tmpParam);
	cmd->SetParam(tmpParam);

	constexpr bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(cmd.release(), isReloadHotKey);

	return true;
}

} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

