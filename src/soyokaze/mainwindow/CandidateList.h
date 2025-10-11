#pragma once

#include <memory>
#include <vector>
#include "commands/core/CommandIF.h"
#include "CandidateListListenerIF.h"

class CandidateList
{
public:
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

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

