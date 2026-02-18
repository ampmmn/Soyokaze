#include "pch.h"
#include "UIElementAliasMap.h"
#include "utility/Path.h"
#include "utility/LocalDirectoryWatcher.h"
#include <nlohmann/json.hpp>
#include <mutex>
#include <fstream>

using json = nlohmann::json;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace uiautomation {

struct UIElementAliasMap::PImpl
{
	void Load();
	void Save();

	void Merge();

	// 排他制御用
	std::mutex mMutex;
	// 関連付けを保持するためのJSONオブジェクト
	json mNameMap;
	// 初回ロード済フラグ
	bool mIsLoaded{false};
	// 状態変更かあったかどうかを示すフラグ
	bool mIsUpdated{false};
};

void UIElementAliasMap::PImpl::Load()
{
	Path path(Path::APPDIR, _T("uielement-menu-alias.json"));
	if (path.FileExists() == false) {
		return ;
	}

	// ファイルが更新されたら通知を受け取るための登録をする
	LocalDirectoryWatcher::GetInstance()->Register((LPCTSTR)path, [](void* p) {

			// ファイルを読み込み、現在の内容とマージする
			auto thisPtr = (UIElementAliasMap::PImpl*)p;
			std::lock_guard<std::mutex> lock(thisPtr->mMutex);
			thisPtr->Merge();
	}, this);

	try {
		std::ifstream f((LPCTSTR)path);
		json obj = json::parse(f);
		mNameMap.swap(obj);
	} catch(const json::exception& e) {
		spdlog::error("json::exception {}", e.what());
	}
}

void UIElementAliasMap::PImpl::Save()
{
	try {
		// ファイルに出力
		Path path(Path::APPDIR, _T("uielement-menu-alias.json"));
		std::ofstream file((LPCTSTR)path);
		file << mNameMap.dump(4);
	} catch(const json::exception& e) {
		spdlog::error("json::exception {}", e.what());
	}
}

void UIElementAliasMap::PImpl::Merge()
{
	try {
		// JSONファイルをロードする
		Path path(Path::APPDIR, _T("uielement-menu-alias.json"));
		std::ifstream f((LPCTSTR)path);
		json obj = json::parse(f);

		// 読み込んだJSONファイルから得た内容と、ロード済のJSONオブジェクトの内容をマージする
		for (auto it = obj.begin(); it != obj.end(); ++it) {

			std::string key = it.key();
			auto dictSrc = it.value();

			// JSONファイルにあったウインドウ(クラス)のキーが存在しない場合(通常はない)
			auto itDst = mNameMap.find(key);
			if (itDst == mNameMap.end()) {
				mNameMap[key] = dictSrc;
				mIsUpdated = true;
				continue;
			}

			auto& dictDst = itDst.value();
			for (auto it2 = dictSrc.begin(); it2 != dictSrc.end(); ++it2) {

				// マージ元(ファイル側)に定義されているエントリ
				auto orgMenuName = it2.key();

				auto& srcEntry = it2.value();
				auto aliasName = srcEntry["alias"].get<std::string>();

				// マージ先側にあるか調べる
				auto it3 = dictDst.find(orgMenuName);
				if (it3 == dictDst.end()) {
					// ないので丸ごと上書き
					mIsUpdated = true;
					dictDst[orgMenuName] = srcEntry;
					continue;
				}

				auto& dstEntry = it3.value();
				if (dstEntry["alias"].get<std::string>() != aliasName) {
					// aliasが更新されているので反映する
					mIsUpdated = true;
					dictDst[orgMenuName] = srcEntry;
					continue;
				}
			}
		}

	} catch(const json::exception& e) {
		spdlog::error("json::exception {}", e.what());
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



UIElementAliasMap::UIElementAliasMap() : in(new PImpl)
{
}

UIElementAliasMap::~UIElementAliasMap()
{
}

UIElementAliasMap* UIElementAliasMap::GetInstance()
{
	static UIElementAliasMap inst;
	return &inst;
}

void UIElementAliasMap::Update()
{
	std::lock_guard<std::mutex> lock(in->mMutex);

	// 初回のみ読み込みを行う
	if (in->mIsLoaded == false) {
		in->Load();
		in->mIsLoaded = true;
		return;
	}
	// エントリの更新があったら、ファイルに書き出す
	if (in->mIsUpdated) {
		in->Save();
		in->mIsUpdated = false;
	}
}

// メニュー名に関連付けられたエイリアス(別名)があったら取得する
bool UIElementAliasMap::GetAlias(
	const CString& wndClassName,
 	const CString& orgMenuName,
	UINT commandId,
 	CString& alias
)
{
	try {
		std::string clsName;
		UTF2UTF(wndClassName, clsName);

		std::lock_guard<std::mutex> lock(in->mMutex);

		// 登録元ウインドウ(クラス)の情報がない場合は更新する
		auto it = in->mNameMap.find(clsName);
		if (it == in->mNameMap.end()) {
			in->mIsUpdated = true;
			in->mNameMap[clsName] = json::object();
			it = in->mNameMap.find(clsName);
		}

		std::string name;
		UTF2UTF(orgMenuName, name);

		// 元の名前の情報がJSON側に存在しない場合は登録する
		auto& dict = it.value();
		auto it2 = dict.find(name);
		if (it2 == dict.end()) {
			in->mIsUpdated = true;
			dict[name] = json{ {"alias",""}, {"command-id", commandId} };
			return false;
		}

		// JSONオブジェクト側に別名の情報があったら取得する
		auto& entry = it2.value();
		auto value = entry["alias"].get<std::string>();
		if (value.empty()) {
			return false;
		}

		UTF2UTF(value, alias);
		return true;

	} catch(const json::exception& e) {
		spdlog::error("json::exception {}", e.what());
		return false;
	}
}

void UIElementAliasMap::EnumElements(
	const CString& wndClassName,
 	std::vector<std::pair<UINT,CString> >& elements
)
{
	try {
		std::string clsName;
		UTF2UTF(wndClassName, clsName);

		std::lock_guard<std::mutex> lock(in->mMutex);

		auto it = in->mNameMap.find(clsName);
		if (it == in->mNameMap.end()) {
			return ;
		}

		CString tmp;

		auto& dict = it.value();
		for (auto it2 = dict.begin(); it2 != dict.end(); ++it2) {

			std::string orgMenuName = it2.key();
			auto& entry = it2.value();
			elements.push_back(std::make_pair(entry["command-id"].get<int>(), UTF2UTF(orgMenuName, tmp)));
		}
	} catch(const json::exception& e) {
		spdlog::error("json::exception {}", e.what());
	}
}

}}} // end of namespace launcherapp::commands::uiautomation


