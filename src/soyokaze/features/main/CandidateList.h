#pragma once

#include <memory>
#include <vector>
#include <cstddef>
#include "commands/core/CommandIF.h"
#include "CandidateListListenerIF.h"

class CandidateList
{
public:
	struct Statistics
	{
		size_t candidateCount{0};
		size_t listenerCount{0};
		size_t errorCommandCount{0};
	};

	CandidateList();
	~CandidateList();

	void SetItems(std::vector<RefPtr<launcherapp::core::Command> >& items);

	int GetCurrentSelect();

	bool SetCurrentSelect(int index);
	bool OffsetCurrentSelect(int index, bool isLoop = true);

	bool IsEmpty();
	int GetSize();
	launcherapp::core::Command* GetCommand(int index);
	launcherapp::core::Command* GetCurrentCommand();

	// 現在選択している項目の説明を取得する
	CString GetCurrentCommandDescription();

	void Clear();

	void AddListener(CandidateListListenerIF* listener);
	void RemoveListener(CandidateListListenerIF* listener);

	// 登録されているリスナー数を取得する。
	size_t GetListenerCount();
	// エラー表示用コマンドが生成済みかどうかを取得する。
	bool HasErrorCommand();
	// すべてのCandidateListインスタンスの統計値を取得する。
	static Statistics GetStatistics();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

