#include "pch.h"
#include "framework.h"
#include "ShellExecCommand.h"
#include "AppPreference.h"

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
	return mDescription.IsEmpty() ? mName : mDescription;
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
	for (int i = 0; i < (int)args.size(); ++i) {
		CString key;
		key.Format(_T("$%d"), i+1);

		path.Replace(key, args[i]);
		param.Replace(key, args[i]);
	}
	// ToDo: 環境変数の展開


	SHELLEXECUTEINFO si = SHELLEXECUTEINFO();
	si.cbSize = sizeof(si);
	si.nShow = attrPtr.mShowType;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = path;
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
	return *this;
}

ShellExecCommand& ShellExecCommand::SetAttributeForParam0(const ATTRIBUTE& attr)
{
	mNoParamAttr = attr;
	return *this;
}

ShellExecCommand& ShellExecCommand::SetPath(LPCTSTR path)
{
	mNormalAttr.mPath = path;
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
