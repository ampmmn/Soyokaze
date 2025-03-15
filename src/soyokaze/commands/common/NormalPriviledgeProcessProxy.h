#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace common {

class NormalPriviledgeProcessProxy
{
	NormalPriviledgeProcessProxy();
	~NormalPriviledgeProcessProxy();

public:
	static NormalPriviledgeProcessProxy* GetInstance();

	bool StartProcess(SHELLEXECUTEINFO* si);

// 以下、通常権限で実行されたプロセス側から実行するメソッド
	bool RunAgentUntilParentDie(int argc, TCHAR* argv[]);

private:
	void OnCopyData(COPYDATASTRUCT* data);
	static LRESULT CALLBACK OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}
}


