#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace core {

/**
 * コマンドの順位を保持するためのクラス
 * 数値が高いものほど候補の先頭に表示される
 */
class CommandRanking
{
private:
	CommandRanking();
	~CommandRanking();

public:
	static CommandRanking* GetInstance();

	// 読み込み
	bool Load();
	// 保存
	bool Save();

	// 順位変更
	void Add(const CString& name, int num);
	//
	void Set(const CString& name, int num);
	// 順位取得
	int Get(const CString& name) const;

	// 削除
	bool Delete(const CString& name);

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

