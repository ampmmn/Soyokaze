#include "pch.h"
#include "framework.h"
#include "PyExtensionCommandEditDialog.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "python/PythonDLLLoader.h"
#include "commands/validation/CommandEditValidation.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/Accessibility.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace py_extension {

using namespace launcherapp::commands::validation;


CommandEditDialog::CommandEditDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_PYEXTENSION_EDIT, parentWnd)
{
	SetHelpPageId("PyExtensionEdit");
}

CommandEditDialog::~CommandEditDialog()
{
}

void CommandEditDialog::SetName(const CString& name)
{
	mParam.mName = name;
}

void CommandEditDialog::SetOriginalName(const CString& name)
{
	mOrgName = name;
}

void CommandEditDialog::SetParam(const CommandParam& param)
{
	mParam = param;
}

const CommandParam& CommandEditDialog::GetParam()
{
	return mParam;
}

void CommandEditDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, mParam.mDescription);
	DDX_Text(pDX, IDC_EDIT_SCRIPT, mParam.mScript);
	DDX_Text(pDX, IDC_EDIT_HOTKEY, mHotKey);
	DDX_Text(pDX, IDC_EDIT_RESULT, mResultMsg);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CommandEditDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_SCRIPT, OnScriptChanged)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_COMMAND(IDC_BUTTON_SYNTAXCHECK, OnButtonSyntaxCheck)
	ON_COMMAND(IDC_BUTTON_RUN, OnButtonRun)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL CommandEditDialog::OnInitDialog()
{
	__super::OnInitDialog();

	SetIcon(IconLoader::Get()->LoadDefaultIcon(), FALSE);

	UpdateTitle();

	UpdateStatus();
	UpdateData(FALSE);

	ScopeAttachThreadInput scope;
	SetForegroundWindow();

	return TRUE;
}

BOOL CommandEditDialog::PreTranslateMessage(MSG* msg)
{
	if (msg->message != WM_KEYDOWN || msg->wParam != VK_TAB) {
		return __super::PreTranslateMessage(msg);
	}

	CEdit* scriptEditCtrl = (CEdit*)GetDlgItem(IDC_EDIT_SCRIPT);
	if (scriptEditCtrl && scriptEditCtrl == GetFocus()) {
		// タブ文字を挿入しフォーカス移動を抑制
		scriptEditCtrl->ReplaceSel(_T("\t"));
		return TRUE;
	}

	return __super::PreTranslateMessage(msg);
}

bool CommandEditDialog::UpdateStatus()
{
	mHotKey = mParam.mHotKeyAttr.ToString();
	if (mHotKey.IsEmpty()) {
		mHotKey.LoadString(IDS_NOHOTKEY);
	}

	int errCode;
	bool isValid = mParam.IsValid(mOrgName, &errCode); 

	if (isValid) {
		if (mIsError == false) {
			mMessage.Empty();
			mIsError = false;
		}
	}
	else {
		CommandParamError paramErr(errCode);
		mMessage = paramErr.ToString();
	}

	UpdateTitle();
	GetDlgItem(IDOK)->EnableWindow(isValid && mIsTested);
	return isValid;
}

void CommandEditDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandEditDialog::OnScriptChanged()
{
	if (mIsTested) {
		mIsTested = false;
		UpdateTitle();
	}
}

HBRUSH CommandEditDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (::utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = mIsError ? RGB(255,0,0) : RGB(0, 0, 0);
		pDC->SetTextColor(crTxt);
	}
	return br;
}

void CommandEditDialog::OnOK()
{
	UpdateData();
	if (TestSyntax() == false) {
		UpdateStatus();
		UpdateData(FALSE);
		return ;
	}
	if (UpdateStatus() == false) {
		return ;
	}

	__super::OnOK();
}

void CommandEditDialog::OnButtonHotKey()
{
	UpdateData();

	if (CommandHotKeyDialog::ShowDialog(mParam.mName, mParam.mHotKeyAttr, this) == false) {
		return ;
	}
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandEditDialog::UpdateTitle()
{
	CString caption;
	caption.Format(_T("Python拡張コマンドの編集【%s】%s"), 
	               mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)mOrgName, mIsTested ? _T("") : _T("*"));
	SetWindowText(caption);

}

bool CommandEditDialog::TestSyntax()
{
	auto loader = PythonDLLLoader::Get();
	if (loader->Initialize() == false) {
		mResultMsg = _T("Pythonを利用できません");
		mIsError = true;
		return false;
	}
	auto proxy = loader->GetLibrary();
	if (proxy->IsPyCmdAvailable() == false) {
		mResultMsg = _T("設定されたPythonのバージョンはサポートしていません\n(3.12以降を指定してください)");
		mIsTested = false;
		mIsError = true;
		return false;
	}

	std::string src;
	UTF2UTF(mParam.mScript, src);
	char* errMsg= nullptr;
	if (proxy->CompileTest(src.c_str(), &errMsg) == false) {
		UTF2UTF(errMsg, mResultMsg);
		proxy->ReleaseBuffer(errMsg);
		mMessage = _T("スクリプトを修正して問題を解消してください。修正したら構文チェックボタンを押してください");
		mIsTested = false;
		mIsError = true;
		return false;
	}
	else {
		mResultMsg.Empty();
		mResultMsg = _T("構文チェックOK");
		mIsTested = true;
		mIsError = false;
		return true;
	}
}

void CommandEditDialog::OnButtonSyntaxCheck()
{
	UpdateData();

	TestSyntax();
	UpdateStatus();

	UpdateData(FALSE);
}

void CommandEditDialog::OnButtonRun()
{
	UpdateData();

	if (TestSyntax() == false) {
		UpdateStatus();
		UpdateData(FALSE);
		return;
	}

	auto loader = PythonDLLLoader::Get();
	auto proxy = loader->GetLibrary();


	std::string src;
	UTF2UTF(mParam.mScript, src);
	char* errMsg= nullptr;
	CWaitCursor wc;
	if (proxy->Evaluate(src.c_str(), &errMsg) == false) {
		UTF2UTF(errMsg, mResultMsg);
		proxy->ReleaseBuffer(errMsg);
	}
	else {
		mResultMsg.Empty();
		mIsError = false;
	}

	UpdateData(FALSE);
}

// マニュアル表示
void CommandEditDialog::OnNotifyLinkOpen(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	UNREFERENCED_PARAMETER(pNMHDR);
	*pResult = 0;
	return;
}

}}}

