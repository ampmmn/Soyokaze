#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace common {

class CandidateExclusionList
{
public:
	CandidateExclusionList();
	~CandidateExclusionList();

	/**
	  設定から除外対象のパスと表示名パターンを読み込む
	*/
	void Load();
	/**
	  ファイルパスが除外対象かどうかを判定する
	  @return true:除外対象 false:対象外
	  @param[in] path 判定するファイルパス
	*/
	bool IsExcludedPath(const CString& path) const;
	/**
	  表示名が除外パターンに一致するかどうかを判定する
	  @return true:除外対象 false:対象外
	  @param[in] displayName 判定する表示名
	*/
	bool IsExcludedDisplayName(const CString& displayName) const;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
