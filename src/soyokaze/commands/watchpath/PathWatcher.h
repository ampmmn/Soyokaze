#pragma once

namespace launcherapp {
namespace commands {
namespace watchpath {

class PathWatcher
{
public:
	struct ITEM {
		// 監視対象パス
		CString mPath;
		// 通知メッセージ
		CString mMessage;
		// 通知の間隔(前回の通知から次の通知までの間隔、秒数)
		UINT mInterval;
	};
private:
	PathWatcher();
	~PathWatcher();

public:
	static PathWatcher* Get();

	void RegisterPath(const CString& cmdName, const ITEM& item);
	void UnregisterPath(const CString& cmdName);
	
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

