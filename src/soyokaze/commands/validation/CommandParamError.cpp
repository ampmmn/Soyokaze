#include "pch.h"
#include "CommandParamError.h"
#include "commands/validation/CommandParamErrorCode.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace validation {

#define BEGIN_ERR_MESSAGE_MAP() static bool GetCommandParamErrorMessage(int errCode, CString& msg) { \
	switch(errCode) {
#define DECLARE_ERR_MESSAGE(code, msgstr)  case code: msg = msgstr; return true;
#define DECLARE_ERR_MESSAGE_ID(code, msgid)  case code: return msg.LoadString(msgid) != FALSE;
#define END_ERR_MESSGAGE_MAP() } return false; }

BEGIN_ERR_MESSAGE_MAP()
	DECLARE_ERR_MESSAGE(Common_NoError, _T(""))
	DECLARE_ERR_MESSAGE_ID(Common_NoName, IDS_ERR_NAMEISEMPTY)
	DECLARE_ERR_MESSAGE_ID(Common_NameAlreadyExists, IDS_ERR_NAMEALREADYEXISTS)
	DECLARE_ERR_MESSAGE_ID(Common_NameContainsIllegalChar, IDS_ERR_ILLEGALCHARCONTAINS)
	DECLARE_ERR_MESSAGE(ActivateWindow_CaptionAndClassBothEmpty, _T("ウインドウタイトルかウインドウクラスを入力してください"))
	DECLARE_ERR_MESSAGE(ActivateWindow_CaptionIsInvalid, _T("ウインドウタイトルの指定パターンが正しくありません"))
	DECLARE_ERR_MESSAGE(ActivateWindow_ClassIsInvalid, _T("ウインドウクラスの指定パターンが正しくありません"))
	DECLARE_ERR_MESSAGE(Alias_TextIsEmpty, _T("テキストを入力してください"))
END_ERR_MESSGAGE_MAP()

CommandParamError::CommandParamError() : mErrCode(0)
{
}

CommandParamError::CommandParamError(int errCode) : mErrCode(errCode)
{
}

CommandParamError::~CommandParamError()
{
}

CString CommandParamError::ToString() const
{
	CString msg;
	if (GetCommandParamErrorMessage(mErrCode, msg) == false) {
		spdlog::warn("No error message is mapped to error code: {}", mErrCode);
		return _T("");
	}
	return msg;
}

}}}


