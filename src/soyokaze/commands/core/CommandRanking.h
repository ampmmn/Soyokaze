#pragma once

#include <memory>

namespace launcherapp {

namespace core {
	class Command;
}
namespace commands {
namespace core {

/**
 * コマンドの順位を保持するためのクラス
 * 数値が高いものほど候補の先頭に表示される
 */
class CommandRanking
{
public:
	using Command = launcherapp::core::Command;
private:
	CommandRanking();
	~CommandRanking();

public:
	static CommandRanking* GetInstance();

	// 読み込み
	bool Load();
	// 保存
	bool Save();

	//
	void Set(Command* cmd, int num);
	// 順位取得
	int Get(Command* cmd) const;

	bool Delete(Command* cmd);

	// すべてリセット
	void ResetAll();

	// ファイルパス設定
	void SetFilePath(const CString& path);

	CommandRanking* CloneTemporarily();
	void CopyTo(CommandRanking* dst);
	void Release();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;


};

}
}
}

