#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include "commands/core/CommandEntryIF.h"
#include <memory>
#include <regex>

namespace launcherapp { namespace commands { namespace activate_window {

class CommandParam
{
public:
	// 対象ウインドウの決定方法
	enum WindowSelectionStrategy {
		ByClassAndCaption = 0,      // クラス名とキャプションから選択する
		SelectFromList = 1,        // コマンド実行時に列挙した一覧から選択する
	};

public:
	CommandParam() = default;
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	bool IsValid(LPCTSTR orgName, int* errCode) const;

	bool Save(CommandEntryIF* entry) const;
	bool Load(CommandEntryIF* entry);


	bool CanFindHwnd() const;
	HWND FindHwnd();

	bool BuildRegExp(CString* errMsg = nullptr);
	bool TryBuildRegExp(CString* errMsg = nullptr) const;
	bool BuildCaptionRegExp(CString* errMsg = nullptr);
	bool TryBuildCaptionRegExp(tregex& regExp, CString* errMsg = nullptr) const;
	bool BuildClassRegExp(CString* errMsg = nullptr);
	bool TryBuildClassRegExp(tregex& regExp, CString* errMsg = nullptr) const;

	bool IsMatchCaption(LPCTSTR caption);
	bool IsMatchClass(LPCTSTR clsName);

	bool IsUseRegExp() const;
	bool HasCaptionRegExpr() const;
	bool HasClassRegExpr() const;

	bool IsNotifyIfWindowNotFound() const;



public:
	// コマンド名
	CString mName;
	// コマンドの説明
	CString mDescription;

	// ホットキー設定
	CommandHotKeyAttribute mHotKeyAttr;

	// 対象ウインドウの決定方法
	WindowSelectionStrategy mStragegy{ByClassAndCaption};

// クラス名とキャプションから選択する場合の設定
	// クラス名
	CString mCaptionStr;
	// キャプション
	CString mClassStr;

	// ウインドウの位置とサイズを変更する場合の配置情報
	WINDOWPLACEMENT mPlacement{};

	// クラス名とキャプション指定は正規表現パターン指定か?
	bool mIsUseRegExp{false};
	// ウインドウの位置・サイズを変更する
	bool mShouldArrangeWindow{false};
	// ウインドウをアクティブにする
	bool mShouldActivateWindow{true};
	// ウインドウが見つからなかった場合に通知
	bool mIsNotifyIfWindowNotFound{false};
	// 自動実行を許可するか?(クラス名とキャプションから選択する場合のみ)
	bool mIsAllowAutoExecute{false};
	// ホットキーからのみ利用する
	bool mIsHotKeyOnly{false};

private:
	std::unique_ptr<tregex> mRegClass;
	std::unique_ptr<tregex> mRegCaption;

};



}}}

