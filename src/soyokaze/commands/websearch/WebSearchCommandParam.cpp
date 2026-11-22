#include "pch.h"
#include "WebSearchCommandParam.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace websearch {

CommandParam::CommandParam() :
	mIsEnableShortcut(false)
{
}

CommandParam::~CommandParam()
{
}

bool CommandParam::Save(CommandEntryIF* entry)
{
	entry->Set(_T("description"), mDescription);

	entry->Set(_T("URL"), mURL);
	entry->Set(_T("IsEnableShortcut"), mIsEnableShortcut);
	// 検索ウインドウの表示方法とタイトルを保存する
	entry->Set(_T("IsUseSearchViewer"), mIsUseSearchViewer);
	entry->Set(_T("SearchViewerTitle"), mSearchViewerTitle);
	entry->SetBytes(_T("IconData"), (const uint8_t*)mIconData.data(), mIconData.size());

	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));
	mURL = entry->Get(_T("URL"), _T(""));
	mIsEnableShortcut = entry->Get(_T("IsEnableShortcut"), false);
	// 古い設定ファイルには検索ウインドウ設定がないため、既定ブラウザーを選ぶ
	mIsUseSearchViewer = entry->Get(_T("IsUseSearchViewer"), false);
	mSearchViewerTitle = entry->Get(_T("SearchViewerTitle"), _T(""));

	size_t len = entry->GetBytesLength(_T("IconData"));
	if (len != CommandEntryIF::NO_ENTRY) {
		mIconData.resize(len);
		entry->GetBytes(_T("IconData"), (uint8_t*)mIconData.data(), len);
	}
	return true;
}

bool CommandParam::IsEnableShortcutSearch() const
{
	return mIsEnableShortcut;
}

}
}
}

