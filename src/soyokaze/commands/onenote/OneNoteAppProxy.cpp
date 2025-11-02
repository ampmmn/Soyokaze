#include "pch.h"
#include "OneNoteAppProxy.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"

using json = nlohmann::json;
using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;

namespace launcherapp { namespace commands { namespace onenote {

struct OneNoteAppProxy::PImpl
{
	CTime StrToCTime(const wchar_t* str) {
		int year, month, day, hour, minute, second;
		swscanf_s(str, L"%4d-%2d-%2dT%2d:%2d:%2d", &year, &month, &day, &hour, &minute, &second);

		CTime utcTime(year, month, day, hour, minute, second);
		return CTime(utcTime.GetTime());
	}
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

bool OneNoteAppProxy::GetHierarchy(std::vector<OneNoteBook>& books)
{
	json json_req;
	json_req["command"] = "onenote_gethierarchy";

	auto proxy = NormalPriviledgeProcessProxy::GetInstance();

	// リクエストを送信する
	json json_res;
	if (proxy->SendRequest(json_req, json_res) == false) {
		return false;
	}
	
	if (json_res["result"] == false) {
		return false;
	}

	std::vector<OneNoteBook> tmpBooks;

	std::wstring tmp;

	// ここでjson_res経由で結果を取り出す
	auto jsonBooks = json_res["books"];
	for(auto book : jsonBooks) {

		OneNoteBook oneNoteBook;
		UTF2UTF(book["ID"].get<std::string>(), oneNoteBook.mID);
		UTF2UTF(book["name"].get<std::string>(), oneNoteBook.mName);
		UTF2UTF(book["nickname"].get<std::string>(), oneNoteBook.mNickName);

		UTF2UTF(book["lastModifiedTime"].get<std::string>(), tmp);
		oneNoteBook.mLastModified = in->StrToCTime(tmp.c_str());

		auto sections = book["sections"];
		for (auto section : sections) {

			OneNoteSection oneNoteSection;
			UTF2UTF(section["ID"].get<std::string>(), oneNoteSection.mID);
			UTF2UTF(section["name"].get<std::string>(), oneNoteSection.mName);
			UTF2UTF(section["lastModifiedTime"].get<std::string>(), tmp);
			oneNoteSection.mLastModifiedTime = in->StrToCTime(tmp.c_str());

			auto pages = section["pages"];
			for (auto page : pages) {

				OneNotePage oneNotePage;
				UTF2UTF(page["ID"].get<std::string>(), oneNotePage.mID);
				UTF2UTF(page["name"].get<std::string>(), oneNotePage.mName);

				UTF2UTF(page["dateTime"].get<std::string>(), tmp);
				oneNotePage.mDateTime = in->StrToCTime(tmp.c_str());
				UTF2UTF(page["lastModifiedTime"].get<std::string>(), tmp);
				oneNotePage.mLastModifiedTime = in->StrToCTime(tmp.c_str());
				oneNotePage.mIsSubPage = page["isSubPage"].get<bool>();

				oneNoteSection.mPages.push_back(oneNotePage);
			}
			oneNoteBook.mSections.push_back(oneNoteSection);
		}

		tmpBooks.push_back(oneNoteBook);
	}


	books.swap(tmpBooks);

	return true;
}

bool OneNoteAppProxy::NavigateTo(LPCWSTR id)
{
	std::string dst;

	json json_req;
	json_req["command"] = "onenote_navigateto";
	json_req["page_id"] = UTF2UTF(std::wstring(id), dst);

	auto proxy = NormalPriviledgeProcessProxy::GetInstance();

	// リクエストを送信する
	json json_res;
	if (proxy->SendRequest(json_req, json_res) == false) {
		return false;
	}

	return json_res["result"];
}



}}} // end of namespace launcherapp::commands::onenote

