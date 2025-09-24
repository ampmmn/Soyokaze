#pragma once

#include "core/UnknownIF.h"
#include "actions/core/ActionParameterIF.h"

namespace launcherapp { namespace actions { namespace core {

class Action : virtual public launcherapp::core::UnknownIF
{
public:
	// アクションの内容を示す名称
	virtual CString GetDisplayName() = 0;
	// アクションを実行する
	virtual bool Perform(Parameter* param, String* errMsg = nullptr) = 0;
	// ガイド欄などに表示するかどうか
	virtual bool IsVisible() = 0;
};



}}}

