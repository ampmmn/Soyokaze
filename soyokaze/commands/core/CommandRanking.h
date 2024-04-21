#pragma once

#include <memory>

namespace launcherapp {
namespace core {

/**
 * コマンドの順位を保持するためのクラス
 * 数値が高いものほど候補の先頭に表示される
 */
class CommandRanking
{
public:
	CommandRanking();
	~CommandRanking();

	// 読み込み
	bool Load();
	// 保存
	bool Save();

	// 順位変更
	void Add(const CString& name, int num);
	// 順位取得
	int Get(const CString& name) const;

	// 削除
	bool Delete(const CString& name);

	// ファイルパス設定
	void SetFilePath(const CString& path);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;


};

}
}

