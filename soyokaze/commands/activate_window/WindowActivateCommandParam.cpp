#include "pch.h"
#include "WindowActivateCommandParam.h"
#include "resource.h"

namespace soyokaze {
namespace commands {
namespace activate_window {

CommandParam::CommandParam() :
	mIsGlobal(false),
	mIsUseRegExp(FALSE)
{
}

CommandParam::~CommandParam()
{
}

bool CommandParam::BuildRegExp(CString* errMsg)
{
	if (BuildCaptionRegExp(errMsg) == false) {
		return false;
	}
	if (BuildClassRegExp(errMsg) == false) {
		return false;
	}
	return true;
}

bool CommandParam::BuildCaptionRegExp(CString* errMsg)
{
	try {
		if (mIsUseRegExp) {
			mRegCaption = tregex(tstring(mCaptionStr));
		}
	}
	catch(std::regex_error& e) {
		CString msg((LPCTSTR)IDS_ERR_INVALIDREGEXP);
		msg += _T("\n");

		CStringA what(e.what());
		msg += _T("\n");
		msg += (CString)what;
		msg += _T("\n");
		msg += mCaptionStr;

		if (errMsg) {
			*errMsg = msg;
		}
		return false;
	}
	return true;
}

bool CommandParam::BuildClassRegExp(CString* errMsg)
{
	try {
		if (mIsUseRegExp) {
			mRegClass = tregex(tstring(mClassStr));
		}
	}
	catch(std::regex_error& e) {
		CString msg((LPCTSTR)IDS_ERR_INVALIDREGEXP);
		msg += _T("\n");

		CStringA what(e.what());
		msg += _T("\n");
		msg += (CString)what;
		msg += _T("\n");
		msg += mClassStr;

		if (errMsg) {
			*errMsg = msg;
		}
		return false;
	}
	return true;
}

bool CommandParam::IsMatchCaption(LPCTSTR caption)
{
	if (IsUseRegExp()) {
		return std::regex_match(tstring(caption), mRegCaption);
	}
	else {
		return mCaptionStr == caption;
	}
}

bool CommandParam::IsMatchClass(LPCTSTR className)
{
	if (IsUseRegExp()) {
		return std::regex_match(tstring(className), mRegClass);
	}
	else {
		return mClassStr == className;
	}
}

bool CommandParam::IsUseRegExp() const
{
	return mIsUseRegExp != FALSE;
}

bool CommandParam::HasCaptionRegExpr() const
{
	return mCaptionStr.IsEmpty() == FALSE;
}

bool CommandParam::HasClassRegExpr() const
{
	return mClassStr.IsEmpty() == FALSE;
}

}
}
}

