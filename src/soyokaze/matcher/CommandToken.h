#pragma once

#include <vector>

namespace launcherapp {
namespace matcher {

class CommandToken
{
public:
	CommandToken(const CString& commandStr);
	~CommandToken() = default;

	bool GetTrailingString(int endPos, CString& trailingText);
	/**
	  指定位置を含むトークンの範囲を取得する
	  @param[in]  position  文字列上の位置
	  @param[out] startPos  トークンの開始位置
	  @param[out] endPos    トークンの終了位置
	  @return トークンが存在する場合はtrue
	*/
	bool GetTokenRange(int position, int& startPos, int& endPos) const;
	/**
	  指定したトークンの文字列を取得する
	  @param[in]  index  トークンのインデックス
	  @param[out] token  トークン文字列
	  @return トークンが存在する場合はtrue
	*/
	bool GetToken(int index, CString& token) const;

	size_t GetCount() const;

protected:
	void EnumTokenPos(std::vector<int>& tokenPos);

private:
	CString mCommandStr;
	std::vector<int> mTokenPos;
};


}
}

