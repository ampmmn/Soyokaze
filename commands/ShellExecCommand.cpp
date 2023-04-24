#include "pch.h"
#include "framework.h"
#include "ShellExecCommand.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ShellExecCommand::ShellExecCommand(
	const CString& name,
 	const CString& path,
 	const CString& param,
 	const CString& dir,
 	const CString& comment,
	int nShowType
) :
	m_strName(name), m_strPath(path), m_strParam(param), m_strDir(dir), m_strComment(comment), m_nShowType(nShowType)
{
}

ShellExecCommand::~ShellExecCommand()
{
}


CString ShellExecCommand::GetDescription()
{
	return m_strComment.IsEmpty() ? m_strName : m_strComment;
}

BOOL ShellExecCommand::Execute()
{
	SHELLEXECUTEINFO si = SHELLEXECUTEINFO();
	si.cbSize = sizeof(si);
	si.nShow = m_nShowType;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = m_strPath;
	if (m_strParam.IsEmpty() == FALSE) {
		si.lpParameters = m_strParam;
	}
	if (m_strDir.IsEmpty() == FALSE) {
		si.lpDirectory = m_strDir;
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

BOOL ShellExecCommand::Match(const CString& strQueryStr)
{
	// 前方一致検索
	return m_strName.Find(strQueryStr) == 0;
}

