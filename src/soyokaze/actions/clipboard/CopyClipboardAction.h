#pragma once

#include "actions/core/ActionBase.h"

namespace launcherapp { namespace actions { namespace clipboard {

// Performで与えられたパラメータの内容をクリップボードにコピーするアクション
class CopyAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	CopyAction();
	~CopyAction();

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;
};

// コンストラクタで与えたテキストの内容をクリップボードにコピーするアクション
class CopyTextAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	CopyTextAction(const CString& text, bool enablePaste = false);
	~CopyTextAction();

	void EnablePasteAfterCopy(bool isEnabled);

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

private:
	// コピーするテキスト
	CString mText;
	// コピーのあと、貼り付けをする
	bool mEnablePaste;
};




}}}

