#include "pch.h"
#include "CandidateList.h"
#include "commands/error/ErrorIndicatorCommand.h"
#include "commands/core/SelectionBehavior.h"
#include "commands/core/IFIDDefine.h"
#include <vector>
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using Command = launcherapp::core::Command;
using ErrorIndicatorCommand = launcherapp::commands::error::ErrorIndicatorCommand;

struct CandidateList::PImpl
{
	void ClearItems()
	{
		for (auto& command : mCandidates) {
			command->Release();
		}
		mCandidates.clear();
	}

	void Unselect()
	{
		size_t index = (size_t)mSelIndex;
		if (index < mCandidates.size()) {
			NotifySelect(mCandidates[index], nullptr);
		}
	}

	void NotifySelect(Command* prior, Command* next)
	{
		RefPtr<launcherapp::core::SelectionBehavior> behaviorP;
		if (prior && prior->QueryInterface(IFID_SELECTIONBEHAVIOR, (void**)&behaviorP)) {
			behaviorP->OnUnselect(next);
		}
		RefPtr<launcherapp::core::SelectionBehavior> behaviorN;
		if (next && next->QueryInterface(IFID_SELECTIONBEHAVIOR, (void**)&behaviorN)) {
			behaviorN->OnSelect(prior);
		}
	}

	// 一覧
	std::vector<Command*> mCandidates;
	// 選択中のもの
	int mSelIndex{-1};

	// 選択したコマンドが「実行不可」の場合に代替で表示するコマンド
	RefPtr<ErrorIndicatorCommand> mErrorCommand;

	// リスナー一覧
	std::set<CandidateListListenerIF*> mListeners;
};

CandidateList::CandidateList() : in(std::make_unique<PImpl>())
{
}

CandidateList::~CandidateList()
{
	in->ClearItems();
}

void CandidateList::SetItems(std::vector<launcherapp::core::Command*>& items)
{
	in->Unselect();
	in->ClearItems();
	in->mCandidates.swap(items);

	in->mSelIndex = 0;

	// 次に選択されるコマンドに対して通知を行う
	auto nextCmd = GetCommand(0);
	in->NotifySelect(nullptr, nextCmd);

	for (auto& listener : in->mListeners) {
		listener->OnUpdateItems((void*)this);
	}
}

int CandidateList::GetCurrentSelect()
{
	return in->mSelIndex;
}

bool CandidateList::SetCurrentSelect(int index)
{
	// 変化しない場合は何もしない
	if (in->mSelIndex == index) {
		return true;
	}

	// 選択が解除されるコマンドと次に選択されるコマンドに対して通知を行う
	auto priorCmd = GetCommand(in->mSelIndex);
	auto nextCmd = GetCommand(index);
	in->NotifySelect(priorCmd, nextCmd);

	in->mSelIndex = index;

	for (auto& listener : in->mListeners) {
		listener->OnUpdateSelect((void*)this);
	}

	return true;
}

bool CandidateList::OffsetCurrentSelect(int offset, bool isLoop)
{
	// 選択位置を移動
	int nextIndex = in->mSelIndex + offset;

	// 範囲外にでる場合は飽和 or ループさせる
	if (nextIndex >= (int)in->mCandidates.size()) {
		// 末尾に達した場合
		nextIndex = isLoop ? 0 : (int)(in->mCandidates.size() - 1);
	}
	else if (nextIndex < 0) {
		// 先頭に達した場合
		nextIndex = isLoop ? (int)(in->mCandidates.size()-1) : 0;
	}

	return SetCurrentSelect(nextIndex);
}

bool CandidateList::IsEmpty()
{
	if (in->mCandidates.empty()) {
		return true;
	}

	if (in->mCandidates.size() > 1) {
		return false;
	}

	// 候補数が1で、名前が空文字の場合はデフォルトコマンドとして扱う
	// (デフォルトコマンドは「コマンドがある」とみなさない)
	auto cmd = in->mCandidates[0];
	ASSERT(cmd);
	return cmd->GetName().IsEmpty() != FALSE;
}

int CandidateList::GetSize()
{
	return (int)in->mCandidates.size();
}

Command* CandidateList::GetCommand(int index)
{
	if (index < 0 || in->mCandidates.size() <= (size_t)index) {
		return nullptr;
	}
	return in->mCandidates[index];
}


Command* CandidateList::GetCurrentCommand()
{
	auto cmd = GetCommand(in->mSelIndex); 
	if (cmd == nullptr || cmd->CanExecute()) {
		return cmd;
	}


	// 実行できない場合はダミーのコマンドで置き換える
	if (in->mErrorCommand.get() == nullptr) {
		in->mErrorCommand.reset(new ErrorIndicatorCommand());
	}

	in->mErrorCommand->SetTarget(cmd);
	return in->mErrorCommand.get();
}

void CandidateList::Clear()
{
	in->Unselect();
	in->ClearItems();
	for (auto& listener : in->mListeners) {
		listener->OnUpdateItems((void*)this);
	}
}

// 現在選択している項目の説明を取得する
CString CandidateList::GetCurrentCommandDescription()
{
	auto command = GetCurrentCommand();
	if (command == nullptr) {
		return _T("");
	}

	auto str = command->GetDescription();
	if (str.IsEmpty()) {
		str = command->GetName();
	}
	return str;
}

void CandidateList::AddListener(CandidateListListenerIF* listener)
{
	in->mListeners.insert(listener);
}

void CandidateList::RemoveListener(CandidateListListenerIF* listener)
{
	auto it = in->mListeners.find(listener);
	if (it != in->mListeners.end()) {
		in->mListeners.erase(it);
	}
}

