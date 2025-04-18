#include "pch.h"
#include "CandidateList.h"
#include <vector>
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using Command = launcherapp::core::Command;

struct CandidateList::PImpl
{
	void ClearItems()
	{
		for (auto& command : mCandidates) {
			command->Release();
		}
		mCandidates.clear();
	}

	// 一覧
	std::vector<launcherapp::core::Command*> mCandidates;
	// 選択中のもの
	int mSelIndex{-1};

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
	in->ClearItems();
	in->mCandidates.swap(items);
	in->mSelIndex = 0;

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
	if (in->mSelIndex == index) {
		return true;
	}

	in->mSelIndex = index;

	for (auto& listener : in->mListeners) {
		listener->OnUpdateSelect((void*)this);
	}

	return true;
}

bool CandidateList::OffsetCurrentSelect(int offset, bool isLoop)
{
	if (offset == 0) {
		return true;
	}

	in->mSelIndex += offset;
	if (in->mSelIndex >= (int)in->mCandidates.size()) {
		in->mSelIndex = isLoop ? 0 : (int)(in->mCandidates.size() - 1);
	}
	else if (in->mSelIndex < 0) {
		in->mSelIndex = isLoop ? (int)(in->mCandidates.size()-1) : 0;
	}

	for (auto& listener : in->mListeners) {
		listener->OnUpdateSelect((void*)this);
	}

	return true;
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
	return GetCommand(in->mSelIndex);
}

void CandidateList::Clear()
{
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

