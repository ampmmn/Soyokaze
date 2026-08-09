#pragma once

#include <functional>
#include <memory>

namespace launcherapp { namespace commands { namespace explorepath {

class ExplorePathFileCache
{
public:
	struct Entry
	{
		CString mName;
		bool mIsDirectory{false};
	};

	using EntryCallback = std::function<void(const Entry&)>;

	ExplorePathFileCache();
	~ExplorePathFileCache();

	// キャッシュを利用してフォルダ内の要素を列挙する。
	// キャッシュ対象外の大きなフォルダは、要素を逐次コールバックへ渡す。
	bool ForEach(const CString& folderPath, const EntryCallback& callback);

	void Clear();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}}} // end of namespace launcherapp::commands::explorepath
