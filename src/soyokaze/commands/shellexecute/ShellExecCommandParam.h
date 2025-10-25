#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include "commands/core/CommandEntryIF.h"
#include <memory>
#include <regex>
#include <map>

namespace launcherapp { namespace commands { namespace shellexecute {

struct ATTRIBUTE {

	int GetShowType() const;
	void SetShowType(int type);

	CString mPath;
	CString mParam;
	CString mDir;
	int mShowType{0};
};

class ActivateWindowParam
{
public:
	ActivateWindowParam& operator = (const ActivateWindowParam& rhs);

	HWND FindHwnd();
	bool IsMatchCaption(LPCTSTR caption);
	bool IsMatchClass(LPCTSTR clsName);
	bool BuildCaptionRegExp(CString* errMsg);
	bool BuildClassRegExp(CString* errMsg);

	bool Save(CommandEntryIF* entry) const;
	bool Load(CommandEntryIF* entry);

	CString mCaptionStr;
	CString mClassStr;
	bool mIsEnable{false};
	bool mIsUseRegExp{false};

private:
	std::unique_ptr<tregex> mRegClass;
	std::unique_ptr<tregex> mRegCaption;
};

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	bool Save(CommandEntryIF* entry) const;
	bool Load(CommandEntryIF* entry);

	// 実行対象のパスはURLか?
	bool IsPathURL() const;

public:
	// コマンド名
	CString mName;
	// 説明
	CString mDescription;

	ATTRIBUTE mNormalAttr;
	ATTRIBUTE mNoParamAttr;

	// 条件に合致するウインドウがある場合はウインドウ切替だけする設定
	ActivateWindowParam mActivateWindowParam;

	// 管理者権限で実行
	BOOL mIsRunAsAdmin;

	// 引数なし版を使うか?
	BOOL mIsUse0;

	// 説明欄の文字列をマッチングに利用するか?
	BOOL mIsUseDescriptionForMatching;

	// 自動実行を許可するか?
	BOOL mIsAllowAutoExecute;

	// アイコンデータ(空の場合はデフォルトアイコンを使用)
	std::vector<uint8_t> mIconData;

	// 環境変数
	std::map<CString, CString> mEnviron;

	// ホットキー
	CommandHotKeyAttribute mHotKeyAttr;
};


}}} // end of namespace launcherapp::commands::shellexecute

