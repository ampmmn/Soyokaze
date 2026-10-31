#pragma once

#include <string>
#include <vector>

namespace launcherapp { namespace commands { namespace clipboardhistory {

struct ClipboardPreviewMatchRange
{
	size_t mStart;
	size_t mLength;
};

/**
  UTF-8テキスト中で検索キーワードが現れる範囲を取得する
  @param[in] text 検索対象のUTF-8テキスト
  @param[in] keywords 検索キーワードのUTF-8文字列一覧
  @return キーワードが現れるUTF-8バイト範囲
*/
std::vector<ClipboardPreviewMatchRange> FindClipboardPreviewMatchRanges(
	const std::string& text,
	const std::vector<std::string>& keywords
);

}}}
