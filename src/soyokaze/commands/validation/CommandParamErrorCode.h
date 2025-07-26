#pragma once

namespace launcherapp { namespace commands { namespace validation {

enum CommandParamErrorCode
{
	// エラーなし
	Common_NoError = 0,
	// [共通] 名前が空
	Common_NoName,
	// [共通] 同名のコマンドがすでに存在する
	Common_NameAlreadyExists,
	// [共通] 使用できない文字がコマンド名に使われている
	Common_NameContainsIllegalChar,
	// [共通] 不明
	Common_UnknownError,
	// [activate_window] ウインドウタイトルとクラスがどちらも空
	ActivateWindow_CaptionAndClassBothEmpty,
	// [activate_window] ウインドウタイトルの指定が不正
	ActivateWindow_CaptionIsInvalid,
	// [activate_window] ウインドウクラスの指定が不正
	ActivateWindow_ClassIsInvalid,
	// 
};



}}}
