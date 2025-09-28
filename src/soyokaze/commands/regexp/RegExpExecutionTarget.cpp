#include "pch.h"
#include "RegExpExecutionTarget.h"
#include "matcher/Pattern.h"

namespace launcherapp { namespace commands { namespace regexp {

RegExpExecutionTarget::RegExpExecutionTarget(int matchLevel, const CommandParam& param) :
 	mMatchLevel(matchLevel), mParam(param)
{
}

// 正規表現コマンド設定の「実行パス」で指定されたパターンを実行時引数の文字列で正規表現置換を行い、
// 置換結果を実行パスとして返す
CString RegExpExecutionTarget::GetPath(Parameter* args)
{
	const ATTRIBUTE& attr = mParam.mNormalAttr;

	try {
		tregex regexObject;
		if (GetRegex(regexObject) == FALSE) {
			spdlog::error("Failed to get regex");
			return attr.mPath;
		}
	
		tstring path_ = std::regex_replace(tstring(args->GetWholeString()), regexObject, (tstring)attr.mPath);
		return path_.c_str();
	}
	catch(std::regex_error& e) {
		spdlog::error("regex_error : {}", e.what());
		return attr.mPath;
	}

}

// 正規表現コマンド設定の「パラメータ」で指定されたパターンを実行時引数の文字列で正規表現置換を行い、
// 置換結果を実行パスとして返す
CString RegExpExecutionTarget::GetParameter(Parameter* args)
{
	const ATTRIBUTE& attr = mParam.mNormalAttr;
	try {
		tstring wholeText = tstring(args->GetWholeString());

		tregex regexObject;
		if (GetRegex(regexObject) == FALSE) {
			spdlog::error("Failed to get regex");
			return attr.mParam;
		}

		
		tstring paramStr_ = std::regex_replace(wholeText, regexObject, (tstring)attr.mParam);
		return paramStr_.c_str();
	}
	catch(std::regex_error& e) {
		spdlog::error("regex_error : {}", e.what());
		return attr.mParam;
	}

}


CString RegExpExecutionTarget::GetWorkDir(Parameter* args)
{
	const ATTRIBUTE& attr = mParam.mNormalAttr;
	try {
		tstring wholeText = tstring(args->GetWholeString());

		tregex regexObject;
		if (GetRegex(regexObject) == FALSE) {
			spdlog::error("Failed to get regex");
			return attr.mParam;
		}

		
		tstring workDir = std::regex_replace(wholeText, regexObject, (tstring)attr.mDir);
		return workDir.c_str();
	}
	catch(std::regex_error& e) {
		spdlog::error("regex_error : {}", e.what());
		return attr.mParam;
	}

}

int RegExpExecutionTarget::GetShowType(Parameter* args)
{
	UNREFERENCED_PARAMETER(args);
	return mParam.mNormalAttr.mShowType;
}

bool RegExpExecutionTarget::GetRegex(tregex& regexObject)
{
	try {
		if (mMatchLevel == Pattern::WholeMatch) {
			regexObject = tregex((LPCTSTR)mParam.mPatternStr);
			return true;
		}
		if (mMatchLevel == Pattern::FrontMatch) {
			regexObject = tregex(fmt::format(_T("{}.*$"), (LPCTSTR)mParam.mPatternStr));
			return true;
		}
		if (mMatchLevel == Pattern::PartialMatch) {
			regexObject = tregex(fmt::format(_T("^.*{}.*$"), (LPCTSTR)mParam.mPatternStr));
			return true;
		}
		return false;
	}
	catch(std::regex_error&) {
		spdlog::error("invalid regex pattern.");
		return false;
	}
}


}}}
