#include "pch.h"
#include "framework.h"
#include "URLDirectoryIndexCommand.h"
#include "commands/url_directoryindex/DirectoryIndexAdhocCommand.h"
#include "core/IFIDDefine.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommandParam.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommandEditor.h"
#include "actions/mainwindow/MainWindowSetTextAction.h"
#include "utility/WinHttp.h"
#include "commands/core/CommandRepository.h"
#include "matcher/PatternInternal.h"
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "features/main/controller/MainWindowController.h"
#include "resource.h"
#include <lexbor/html/html.h>
#include <lexbor/css/css.h>
#include <lexbor/selectors/selectors.h>
#include <shlwapi.h>

#include <map>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "shlwapi.lib")

using namespace launcherapp::commands::common;

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

using LinkItems = std::vector<std::pair<CString, CString> >;

struct LinkExtractContext
{
	const CString& mFinalURL;
	LinkItems& mItems;
	bool mFailed{false};
};

/**
  CSSセレクターで見つかったアンカーからリンク候補を作成する
  @param[in] node      見つかったDOMノード
  @param[in] specificity セレクターの詳細度
  @param[in] contextPtr リンク抽出コンテキスト
  @return LXB_STATUS_OK:継続  その他:抽出失敗
*/
static lxb_status_t OnAnchorFound(lxb_dom_node_t* node, lxb_css_selector_specificity_t specificity, void* contextPtr)
{
	(void)specificity;
	auto& context = *static_cast<LinkExtractContext*>(contextPtr);
	try {
		auto element = lxb_dom_interface_element(node);
		auto hrefAttr = lxb_dom_element_attr_by_name(element, (const lxb_char_t*)"href", 4);
		if (hrefAttr == nullptr) {
			return LXB_STATUS_OK;
		}

		size_t hrefLength = 0;
		const lxb_char_t* hrefText = lxb_dom_attr_value(hrefAttr, &hrefLength);
		CString href;
		if (hrefText != nullptr && hrefLength > 0) {
			UTF2UTF(std::string((const char*)hrefText, hrefLength), href);
		}

		// Apache Directory Indexのソート用リンクは候補に含めない
		if (href.Find(_T("?C=")) == 0) {
			return LXB_STATUS_OK;
		}

		// 絶対URLは最終ページと同一オリジンのものだけを候補にする
		if (launcherapp::utility::IsSameOriginURL(context.mFinalURL, href) == false) {
			return LXB_STATUS_OK;
		}

		CString absoluteHref;
		if (href.Left(2) == _T("//")) {
			CString scheme = context.mFinalURL.Left(context.mFinalURL.Find(_T(':')));
			absoluteHref = scheme + _T(":") + href;
		}
		else if (href.Left(7).CompareNoCase(_T("http://")) == 0 || href.Left(8).CompareNoCase(_T("https://")) == 0) {
			absoluteHref = href;
		}
		else {
			// リダイレクト後のURLを基準に相対リンクを絶対URLへ変換する
			std::vector<TCHAR> combinedURL(32768);
			DWORD combinedURLLength = static_cast<DWORD>(combinedURL.size());
			if (UrlCombine(context.mFinalURL, href, combinedURL.data(), &combinedURLLength, 0) != S_OK) {
				return LXB_STATUS_OK;
			}
			absoluteHref = combinedURL.data();
		}

		if (absoluteHref.IsEmpty()) {
			return LXB_STATUS_OK;
		}

		size_t textLength = 0;
		lxb_char_t* text = lxb_dom_node_text_content(node, &textLength);
		CString displayName;
		if (text != nullptr && textLength > 0) {
			UTF2UTF(std::string((const char*)text, textLength), displayName);
		}
		displayName.Trim();

		// 表示テキストが"Parent Directory"なら親ディレクトリ表記に置き換える
		if (displayName.CompareNoCase(_T("Parent Directory")) == 0) {
			displayName = _T("..");
		}

		context.mItems.emplace_back(absoluteHref, displayName);

		return LXB_STATUS_OK;
	}
	catch (...) {
		context.mFailed = true;
		return LXB_STATUS_ERROR;
	}
}

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
	bool LoadContent(const CString& url, std::vector<BYTE>& content, bool& isHTML, CString& finalURL);
	bool ExtractCandidates(const std::vector<BYTE>& content, const CString& finalURL, LinkItems& items);

	bool IsLoaded()
	{
		return mIsLoaded;
	}

	size_t GetCandidatesCount()
	{
		return mLinkItems.size();
	}

	void ClearCache()
	{
		mLinkCache.clear();
	}
	void Reset()
	{
		mLinkItems.clear();
		mIsLoaded = false;
		mIsLoadOK = false;
	}

	CommandParam mParam;

	LinkItems mLinkItems;
	std::map<CString, LinkItems> mLinkCache;

	CString mSubPath;

	bool mIsLoaded{false};
	bool mIsLoadOK{false};
};

// 候補一覧の生成を開始する
bool URLDirectoryIndexCommand::PImpl::LoadContent(const CString& url, std::vector<BYTE>& content, bool& isHTML, CString& finalURL)
{
	WinHttp http;
	if (mParam.mServerUser.IsEmpty() == FALSE) {
		http.SetServerCredential(mParam.mServerUser, mParam.DecryptServerPassword());
	}
	http.SetProxyType(mParam.mProxyType);
	http.SetProxyCredential(mParam.mProxyHost, mParam.mProxyUser, mParam.DecryptProxyPassword());

	return http.LoadContent(url, content, isHTML, finalURL);
}


// HTMLからリンク一覧を生成する
bool URLDirectoryIndexCommand::PImpl::ExtractCandidates(
	const std::vector<BYTE>& content,
	const CString& finalURL,
	LinkItems& items
)
{
	return URLDirectoryIndexCommand::ExtractHTMLLinks(content, finalURL, items);
}

bool URLDirectoryIndexCommand::ExtractHTMLLinks(
	const std::vector<BYTE>& content,
	const CString& finalURL,
	LinkItems& items
)
{
	auto documentDeleter = [](lxb_html_document_t* document) {
		if (document) {
			lxb_html_document_destroy(document);
		}
	};
	std::unique_ptr<lxb_html_document_t, decltype(documentDeleter)> document(lxb_html_document_create(), documentDeleter);
	if (document == nullptr || lxb_html_document_parse(document.get(), content.data(), content.size()) != LXB_STATUS_OK) {
		return false;
	}

	auto parserDeleter = [](lxb_css_parser_t* parser) {
		if (parser) {
			lxb_css_parser_destroy(parser, true);
		}
	};
	std::unique_ptr<lxb_css_parser_t, decltype(parserDeleter)> parser(lxb_css_parser_create(), parserDeleter);
	if (parser == nullptr || lxb_css_parser_init(parser.get(), nullptr) != LXB_STATUS_OK) {
		return false;
	}

	static const lxb_char_t selectorText[] = "a[href]";
	auto selectorList = lxb_css_selectors_parse(parser.get(), selectorText, sizeof(selectorText) - 1);
	if (selectorList == nullptr || parser->status != LXB_STATUS_OK) {
		if (selectorList) {
			lxb_css_selector_list_destroy_memory(selectorList);
		}
		return false;
	}

	auto selectorListDeleter = [](lxb_css_selector_list_t* list) {
		if (list) {
			lxb_css_selector_list_destroy_memory(list);
		}
	};
	std::unique_ptr<lxb_css_selector_list_t, decltype(selectorListDeleter)> selectorListGuard(selectorList, selectorListDeleter);

	auto selectorsDeleter = [](lxb_selectors_t* selectors) {
		if (selectors) {
			lxb_selectors_destroy(selectors, true);
		}
	};
	std::unique_ptr<lxb_selectors_t, decltype(selectorsDeleter)> selectors(lxb_selectors_create(), selectorsDeleter);
	if (selectors == nullptr || lxb_selectors_init(selectors.get()) != LXB_STATUS_OK) {
		return false;
	}

	LinkItems links;
	LinkExtractContext context{finalURL, links};
	if (lxb_selectors_find(selectors.get(), lxb_dom_interface_node(document.get()), selectorList,
	                       OnAnchorFound, &context) != LXB_STATUS_OK || context.mFailed) {
		return false;
	}

	items.swap(links);
	return true;

}

bool URLDirectoryIndexCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (UserCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}
	if (ifid == IFID_EXTRACANDIDATESOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidateSource*)this;
		return true;
	}
	return false;
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
	// https?から始まる場合はURLとして扱う
	// '/'で始まる場合は絶対パスとして扱う
	if (subPath.Left(1) == _T('/') || subPath.Left(7).CompareNoCase(_T("http://")) == 0 || subPath.Left(8).CompareNoCase(_T("https://")) == 0) {
		in->mSubPath = subPath;
	}
	else {
		// 相対パスの場合はルートから
		if (in->mSubPath.IsEmpty() == FALSE && in->mSubPath.Right(1) != _T('/')) {
			in->mSubPath += _T('/');
		}
		in->mSubPath += subPath;
	}

	in->Reset();
}

CString URLDirectoryIndexCommand::GetSubPath()
{
	return in->mSubPath;
}

void URLDirectoryIndexCommand::LoadCanidates(bool& isHTML)
{
	CString url = in->mParam.CombineURL(in->mSubPath);

	// キャッシュがあればそれを使う
	auto itCache = in->mLinkCache.find(url);
	if (itCache != in->mLinkCache.end()) {
		in->mLinkItems = itCache->second;
		isHTML = true;
	}
	else {
		// なければダウンロード
		std::vector<BYTE> content;
		CString finalURL;
		if (in->LoadContent(url, content, isHTML, finalURL) == false) {
			return;
		}
		// 取得したテキストを解析
		spdlog::debug("extract candidates");
		LinkItems items;
		if (in->ExtractCandidates(content, finalURL, items) == false) {
			return;
		}

		// リンクがなかったらその旨を示すダミー項目を追加する
		if (items.empty()) {
			items.push_back(std::pair<CString, CString>(_T("."), _T("(No links found)")));
		}

		in->mLinkCache[url] = items;
		in->mLinkItems.swap(items);
	}

	in->mIsLoaded = true;
	in->mIsLoadOK = true;
}

CString URLDirectoryIndexCommand::GetName()
{
	return in->mParam.mName;
}


CString URLDirectoryIndexCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString URLDirectoryIndexCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool URLDirectoryIndexCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() != 0) {
		return false;
	}

	// 入力欄を表示し、コマンド名+ " " が入力された状態にする
	auto cmdline(GetName()+ _T(" "));
	*action = new launcherapp::actions::mainwindow::SetTextAction(_T("候補を表示する"), cmdline);
	return true;
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
	return IconLoader::Get()->GetShell32Icon(-63011);
}

int URLDirectoryIndexCommand::Match(Pattern* pattern)
{
	if (pattern->shouldWholeMatch() && pattern->Match(GetName()) == Pattern::WholeMatch) {
		// 内部のコマンド名マッチング用の判定
		return Pattern::WholeMatch;
	}
	else if (pattern->shouldWholeMatch() == false) {

		// 入力欄からの入力で、前方一致するときは候補に出す
		int level = pattern->Match(GetName());
		if (level == Pattern::FrontMatch || level == Pattern::PartialMatch) {
			return level;
		}
		if (level == Pattern::WholeMatch) {

		 	if (pattern->GetWordCount() > 1) {
				return Pattern::HiddenMatch;
			}

			if (in->IsLoaded()) {

				// 候補列挙済の場合、候補が存在する場合は、候補をFilterAdhocCommandとして表示するため、
				// このコマンド自身は表示しない
				if (in->GetCandidatesCount() > 0) {
					// 候補一覧生成済の場合は表示しない
					return Pattern::Mismatch;
				}

				return Pattern::WholeMatch;
			}

			// このタイミングで候補一覧の生成を行う
			bool isHTML = false;
			LoadCanidates(isHTML);

			return Pattern::WholeMatch;
		}
	}

	// 通常はこちら
	return Pattern::Mismatch;
}

bool URLDirectoryIndexCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
URLDirectoryIndexCommand::Clone()
{
	auto clonedObj = make_refptr<URLDirectoryIndexCommand>();
	clonedObj->in->mParam = in->mParam;
	return clonedObj.release();
}

bool URLDirectoryIndexCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	return in->mParam.Save(entry);
}

bool URLDirectoryIndexCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != URLDirectoryIndexCommand::GetType()) {
		return false;
	}

	in->mParam.Load(entry);

	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(GetName(), &in->mParam.mHotKeyAttr);

	return true;
}

bool URLDirectoryIndexCommand::NewDialog(Parameter* param, URLDirectoryIndexCommand** newCmd)
{
	// 新規作成ダイアログを表示
	CString value;

	CommandParam tmpParam;

	GetNamedParamString(param, _T("COMMAND"), tmpParam.mName);
	GetNamedParamString(param, _T("PATH"), tmpParam.mURL);
	GetNamedParamString(param, _T("DESCRIPTION"), tmpParam.mDescription);

	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	cmdEditor->SetParam(tmpParam);

	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto cmd = make_refptr<URLDirectoryIndexCommand>();

	cmd->SetParam(cmdEditor->GetParam());

	if (newCmd) {
		*newCmd = cmd.release();
	}

	return true;
}

// コマンドを編集するためのダイアログを作成/取得する
bool URLDirectoryIndexCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
{
	if (editor == nullptr) {
		return false;
	}

	auto cmdEditor = new CommandEditor(CWnd::FromHandle(parent));
	cmdEditor->SetParam(in->mParam);

	*editor = cmdEditor;
	return true;
}

// ダイアログ上での編集結果をコマンドに適用する
bool URLDirectoryIndexCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_URLDIRECTORYINDEXCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	in->mParam = cmdEditor->GetParam();

	// 設定変更を反映するため、候補のキャッシュを消す
	ClearCache();
	in->Reset();

	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool URLDirectoryIndexCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_URLDIRECTORYINDEXCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	auto paramNew = cmdEditor->GetParam();

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<URLDirectoryIndexCommand>();
	newCmd->SetParam(paramNew);

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}


bool URLDirectoryIndexCommand::QueryCandidates(Pattern* pattern, CommandQueryItemList& commands)
{
	// コマンド名が一致しなければ候補を表示しない
	if (GetName().CompareNoCase(pattern->GetFirstWord()) != 0) {
		in->Reset();
		in->mSubPath.Empty();
		return false;
	}

	RefPtr<PatternInternal> pat2;
	if (pattern->QueryInterface(IFID_PATTERNINTERNAL, (void**)&pat2) == false) {
		spdlog::info(_T("failed to get IFID_PATTERNINTERNAL ptr."));
		return false;
	}

	DirectoryIndexQueryResult results;

	std::vector<CString> words;
	pat2->GetRawWords(words);
	if (words.size() == 1) {
		// 入力キーワードが空文字の場合は全てを候補に追加する
		for(auto& item : in->mLinkItems) {
			QueryResult result;
			result.mLinkPath = item.first;
			result.mLinkText = item.second;
			result.mURL = in->mParam.CombineURL(item.first);
			result.mMatchLevel = Pattern::FrontMatch;
			results.push_back(result);
		}
	}
	else {
		for(auto& item : in->mLinkItems) {
			int level = pattern->Match(item.second, 1);
			if (level == Pattern::Mismatch) {
				continue;
			}
			if (level == Pattern::PartialMatch) {
				// 先行キーワードは一致しているので前方一致扱いとする
				level = Pattern::FrontMatch;
			}

			QueryResult result;
			result.mLinkPath = item.first;
			result.mLinkText = item.second;
			result.mURL = in->mParam.CombineURL(item.first);
			result.mMatchLevel = level;

			results.push_back(result);
		}
	}

	for (auto& result : results) {
		commands.Add(CommandQueryItem(result.mMatchLevel, new DirectoryIndexAdhocCommand(this, result)));
	}

	return true;
}

void URLDirectoryIndexCommand::ClearCache()
{
	in->ClearCache();
}

CString URLDirectoryIndexCommand::TypeDisplayName()
{
	static CString TEXT_TYPE(_T("DirectiryIndex"));
	return TEXT_TYPE;
}

} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

