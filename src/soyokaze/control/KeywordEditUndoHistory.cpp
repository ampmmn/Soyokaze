#include "pch.h"
#include "KeywordEditUndoHistory.h"
#include <deque>

namespace {
	constexpr size_t MAX_UNDO_HISTORY_SIZE = 16;
}

struct KeywordEditUndoHistory::PImpl
{
	std::deque<CString> mUndoTexts;
	CString mCurrentText;
	bool mIsInitialized{false};
};

KeywordEditUndoHistory::KeywordEditUndoHistory() : in(std::make_unique<PImpl>())
{
}

KeywordEditUndoHistory::~KeywordEditUndoHistory()
{
}

void KeywordEditUndoHistory::Reset(const CString& text)
{
	in->mUndoTexts.clear();
	in->mCurrentText = text;
	in->mIsInitialized = true;
}

bool KeywordEditUndoHistory::RecordChange(const CString& text)
{
	if (in->mIsInitialized == false) {
		Reset(text);
		return false;
	}
	if (in->mCurrentText == text) {
		return false;
	}

	// 変更前の文字列を記録し、古い履歴から上限を超えた分を破棄する
	in->mUndoTexts.push_back(in->mCurrentText);
	if (in->mUndoTexts.size() > MAX_UNDO_HISTORY_SIZE) {
		in->mUndoTexts.pop_front();
	}
	in->mCurrentText = text;
	return true;
}

bool KeywordEditUndoHistory::Undo(CString& text)
{
	if (in->mUndoTexts.empty()) {
		return false;
	}
	text = in->mUndoTexts.back();
	in->mUndoTexts.pop_back();
	in->mCurrentText = text;
	return true;
}

