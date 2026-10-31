#pragma once

#include <algorithm>

namespace launcherapp {
namespace mainwindow {
namespace preview {

struct PreviewLayout
{
	int mCandidateWidth{0};
	int mPreviewX{0};
	int mPreviewWidth{0};
	bool mIsPreviewVisible{false};
};

/**
  クライアント幅と外周マージンから候補欄とプレビュー欄の幅を計算する
  @param[in] clientWidth クライアント領域の幅
  @param[in] marginLeft 左側のマージン
  @param[in] marginRight 右側のマージン
  @return 各欄の位置と幅
*/
inline PreviewLayout CalculatePreviewLayout(int clientWidth, int marginLeft, int marginRight)
{
	PreviewLayout result;
	int availableWidth = (std::max)(0, clientWidth - marginLeft - marginRight);
	if (clientWidth <= 640) {
		result.mCandidateWidth = availableWidth;
		result.mPreviewX = clientWidth;
		return result;
	}

	int candidatePaneWidth = clientWidth < 1280 ? 640 : clientWidth / 2;
	int previewWidth = clientWidth - candidatePaneWidth - marginRight;
	if (previewWidth <= 0) {
		result.mCandidateWidth = availableWidth;
		result.mPreviewX = clientWidth;
		return result;
	}

	result.mCandidateWidth = (std::max)(0, candidatePaneWidth - marginLeft);
	result.mPreviewX = candidatePaneWidth;
	result.mPreviewWidth = previewWidth;
	result.mIsPreviewVisible = true;
	return result;
}

}
}
}
