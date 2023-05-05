#include "pch.h"
#include "framework.h"
#include "ShellExecCommand.h"
#include "AppPreference.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ShellExecCommand::ATTRIBUTE::ATTRIBUTE() :
	mShowType(SW_NORMAL)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


ShellExecCommand::ShellExecCommand()
{
}

ShellExecCommand::~ShellExecCommand()
{
}

CString ShellExecCommand::GetName()
{
	return mName;
}


CString ShellExecCommand::GetDescription()
{
	return mDescription;
}

BOOL ShellExecCommand::Execute()
{
	std::vector<CString> argsEmpty;
	return Execute(argsEmpty);
}

BOOL ShellExecCommand::Execute(const std::vector<CString>& args)
{
	// パラメータあり/なしで、mNormalAttr/mNoParamAttrを切り替える
	const ATTRIBUTE& attrPtr = SelectAttribute(args);

	CString path;
	CString param;
	// Ctrlキーがおされて、かつ、パスが存在する場合はファイラーで表示
	bool isOpenPath = (GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
	                  PathFileExists(attrPtr.mPath);

	if (isOpenPath || PathIsDirectory(attrPtr.mPath)) {

		// 登録されたファイラーで開く
		AppPreference pref;
		pref.Load();

		path = pref.GetFilerPath();
		param = pref.GetFilerParam();
		// とりあえずリンク先のみをサポート
		param.Replace(_T("$target"), attrPtr.mPath);
	}
	else {
		path = attrPtr.mPath;
		param = attrPtr.mParam;
	}

	// argsの値を展開
	ExpandArguments(args, path, param);

	SHELLEXECUTEINFO si = SHELLEXECUTEINFO();
	si.cbSize = sizeof(si);
	si.nShow = attrPtr.mShowType;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = path;
	if (mRunAs == 1) {
		si.lpVerb = _T("runas");
	}

	if (param.IsEmpty() == FALSE) {
		si.lpParameters = param;
	}
	if (attrPtr.mDir.IsEmpty() == FALSE) {
		si.lpDirectory = attrPtr.mDir;
	}
	BOOL bRun = ShellExecuteEx(&si);
	if (bRun == FALSE) {
		return FALSE;
	}

	CloseHandle(si.hProcess);

	return TRUE;
}

CString ShellExecCommand::GetErrorString()
{
	return _T("");
}

ShellExecCommand& ShellExecCommand::SetName(LPCTSTR name)
{
	mName = name;
	return *this;
}

ShellExecCommand& ShellExecCommand::SetDescription(LPCTSTR description)
{
	mDescription = description;
	return *this;
}


ShellExecCommand& ShellExecCommand::SetAttribute(const ATTRIBUTE& attr)
{
	mNormalAttr = attr;
	ExpandEnv(mNormalAttr.mPath);
	ExpandEnv(mNormalAttr.mParam);

	return *this;
}

ShellExecCommand& ShellExecCommand::SetAttributeForParam0(const ATTRIBUTE& attr)
{
	mNoParamAttr = attr;
	ExpandEnv(mNoParamAttr.mPath);
	ExpandEnv(mNoParamAttr.mParam);
	return *this;
}

ShellExecCommand& ShellExecCommand::SetPath(LPCTSTR path)
{
	mNormalAttr.mPath = path;
	ExpandEnv(mNormalAttr.mPath);
	return *this;
}

ShellExecCommand& ShellExecCommand::SetRunAs(int runAs)
{
	mRunAs = runAs;
	return *this;
}

ShellExecCommand::ATTRIBUTE&
ShellExecCommand::SelectAttribute(
	const std::vector<CString>& args
)
{
	// パラメータの有無などでATTRIBUTEを切り替える

	if (args.size() > 0) {
		// パラメータあり

		// mNormalAttr優先
		if (mNormalAttr.mPath.IsEmpty() == FALSE) {
			return mNormalAttr;
		}

		return mNoParamAttr;
	}
	else {
		// パラメータなし

		// mNoParamAttr優先
		if (mNoParamAttr.mPath.IsEmpty() == FALSE) {
			return mNoParamAttr;
		}

		return mNormalAttr;
	}
}

void ShellExecCommand::ExpandArguments(
	const std::vector<CString>& args,
	CString& path,
	CString& param
)
{
	CString argAll;
	for (int i = 0; i < (int)args.size(); ++i) {
		CString key;
		key.Format(_T("$%d"), i+1);

		path.Replace(key, args[i]);
		param.Replace(key, args[i]);

		if (i != 0) {
			argAll += _T(" ");
		}
		argAll += args[i];
	}


	if (path.Find(_T("$*")) != -1) {
		path.Replace(_T("$*"), argAll);
	}
	if (param.Find(_T("$*")) != -1) {
		param.Replace(_T("$*"), argAll);
	}
}

void ShellExecCommand::ExpandEnv(CString& text)
{
	CString workBuf(text);

	int len = text.GetLength();
	for (int i = 0; i < len; ++i) {
		TCHAR c = text[i];

		if (c != _T('$')) {
			continue;
		}
		if (i + 1 >= len) {
			break;
		}

		int startPos = i + 1;

		for (int j = startPos; j < len; ++j) {

			c = text[j];

			bool isa = (_T('a') <= c && c <= _T('z'));
			bool isA = (_T('A') <= c && c <= _T('Z'));
			bool isnum = (_T('0') <= c && c <= _T('9'));
			bool is_ = (c == _T('_') || c == _T('(') || c == _T(')'));

			if (isa || isA || isnum || is_) {
				continue;
			}

			CString valName = text.Mid(startPos, j - startPos);
			if (valName.IsEmpty()) {
				i = j - 1;
				startPos = -1;
				break;
			}

			size_t reqLen = 0;
			if (_tgetenv_s(&reqLen, NULL, 0, valName) != 0 || reqLen == 0) {
				i = j - 1;
				startPos = -1;
				break;
			}

			CString val;
			TCHAR* p = val.GetBuffer((int)reqLen);
			_tgetenv_s(&reqLen, p, reqLen, valName);
			val.ReleaseBuffer();

			CString before(_T("$"));
			before += valName;

			workBuf.Replace(before, val);
			startPos = -1;

			i = j - 1;
			break;
		}
		if (startPos == -1) {
			continue;
		}

		CString valName = text.Mid(startPos, len - startPos);
		if (valName.IsEmpty()) {
			break;
		}

		size_t reqLen = 0;
		if (_tgetenv_s(&reqLen, NULL, 0, valName) != 0 || reqLen == 0) {
			break;
		}

		CString val;
		TCHAR* p = val.GetBuffer((int)reqLen);
		_tgetenv_s(&reqLen, p, reqLen, valName);
		val.ReleaseBuffer();

		CString before(_T("$"));
		before += valName;

		workBuf.Replace(before, val);
		break;
	}

	text = workBuf;
}

HICON ShellExecCommand::GetIcon()
{
	const CString& path = mNormalAttr.mPath;
	return IconLoader::Get()->LoadIconFromPath(path);
}

BOOL ShellExecCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

void ShellExecCommand::GetAttribute(ATTRIBUTE& attr)
{
	attr = mNormalAttr;
}

void ShellExecCommand::GetAttributeForParam0(ATTRIBUTE& attr)
{
	attr = mNoParamAttr;
}

int ShellExecCommand::GetRunAs()
{
	return mRunAs;
}

