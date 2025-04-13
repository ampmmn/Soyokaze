#pragma once

#include <memory>
#include <map>
#include <string>

namespace launcherapp {
namespace commands {
namespace common {

class NormalPriviledgeProcessProxy
{
	NormalPriviledgeProcessProxy();
	~NormalPriviledgeProcessProxy();

public:
	static NormalPriviledgeProcessProxy* GetInstance();

	// 通常権限でコマンドを実行する
	bool StartProcess(SHELLEXECUTEINFO* si, const std::map<std::wstring, std::wstring>& envMap);
	// あふwのカレントディレクトリを取得する
	bool GetCurrentAfxwDir(std::wstring& path);
	// あふwのカレントディレクトリを設定する
	bool SetCurrentAfxwDir(const std::wstring& path);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}
}


