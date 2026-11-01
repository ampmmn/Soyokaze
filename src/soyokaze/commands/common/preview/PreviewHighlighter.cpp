#include "pch.h"
#include "PreviewHighlighter.h"
#include <algorithm>

namespace launcherapp { namespace commands { namespace common { namespace preview {

namespace {

/** ASCIIアルファベットを大文字小文字を区別せずに比較する */
bool IsSameCharacterIgnoreAsciiCase(unsigned char lhs, unsigned char rhs)
{
	if (lhs >= 'A' && lhs <= 'Z') {
		lhs += 'a' - 'A';
	}
	if (rhs >= 'A' && rhs <= 'Z') {
		rhs += 'a' - 'A';
	}
	return lhs == rhs;
}

}

std::vector<PreviewMatchRange> FindPreviewMatchRanges(
	const std::string& text,
	const std::vector<std::string>& keywords
)
{
	std::vector<PreviewMatchRange> ranges;
	for (const auto& keyword : keywords) {
		if (keyword.empty()) {
			continue;
		}

		auto start = text.cbegin();
		while (start != text.cend()) {
			auto found = std::search(start, text.cend(), keyword.cbegin(), keyword.cend(), IsSameCharacterIgnoreAsciiCase);
			if (found == text.cend()) {
				break;
			}

			auto offset = static_cast<size_t>(std::distance(text.cbegin(), found));
			ranges.push_back({offset, keyword.length()});
			start = found + 1;
		}
	}

	std::sort(ranges.begin(), ranges.end(), [](const auto& lhs, const auto& rhs) {
		if (lhs.mStart != rhs.mStart) {
			return lhs.mStart < rhs.mStart;
		}
		return lhs.mLength < rhs.mLength;
	});
	ranges.erase(std::unique(ranges.begin(), ranges.end(), [](const auto& lhs, const auto& rhs) {
		return lhs.mStart == rhs.mStart && lhs.mLength == rhs.mLength;
	}), ranges.end());

	return ranges;
}

}}}}
