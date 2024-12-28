#pragma once

#include "commands/core/UnknownIF.h"

namespace launcherapp {
namespace core {

class CommandEditor : virtual public UnknownIF
{
public:
	// 名前を上書きする
	virtual void OverrideName(LPCTSTR name) = 0;
	// 元のコマンド名を設定する(そのコマンド名と同じ場合は「コマンド名重複」とみなさない)
	virtual void SetOriginalName(LPCTSTR name) = 0;
	// コマンドを編集するためのダイアログを作成/取得する
	virtual bool DoModal() = 0;
};

}  // end of namespace core
}  // end of namespace launcherapp



