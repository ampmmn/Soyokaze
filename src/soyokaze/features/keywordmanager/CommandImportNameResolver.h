#pragma once

#include <functional>

namespace launcherapp { namespace core {

class CommandImportNameResolver
{
public:
	/**
	  重複しない連番付きコマンド名を取得する
	  @param[in]  originalName 元のコマンド名
	  @param[in]  isOccupied   名前が使用済みかを判定する関数
	  @return 重複しない名前
	*/
	static CString GetUniqueName(const CString& originalName, const std::function<bool(const CString&)>& isOccupied);
};

}}
