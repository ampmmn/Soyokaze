#pragma once

#include "commands/core/UnknownIF.h"

namespace launcherapp {
namespace core {

class CommandEditor : virtual public UnknownIF
{
public:
	// コマンドは編集可能か?
	virtual void SetOriginalName(LPCTSTR name) = 0;
	// コマンドを編集するためのダイアログを作成/取得する
	virtual bool DoModal() = 0;
};

}  // end of namespace core
}  // end of namespace launcherapp



