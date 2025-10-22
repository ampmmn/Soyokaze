#include "pch.h"
#include "framework.h"
#include "PyExtensionCommandEditDialog.h"
#include "commands/py_extension/ScintillaDLLLoader.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "python/PythonDLLLoader.h"
#include "commands/validation/CommandEditValidation.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/Accessibility.h"
#include "utility/Path.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>
#include "Scintilla.h"
#include "SciLexer.h"

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
	//DDX_Text(pDX, IDC_EDIT_SCRIPT, mParam.mScript);
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

	auto editCtrl = GetDlgItem(IDC_EDIT_SCRIPT);

	// pythonのシンタックスハイライト
	auto pyLexer = ScintillaDLLLoader::GetInstance()->CreateLexer("python");
	editCtrl->SendMessage(SCI_SETILEXER, 0, (LPARAM)pyLexer);

	editCtrl->SendMessage(SCI_SETKEYWORDS, 0, (LPARAM)
			"and as assert break class continue def del elif else except "
			"False finally for from global if import in is lambda None "
			"nonlocal not or pass raise return True try while with yield");

	editCtrl->SendMessage(SCI_STYLESETFORE, SCE_P_WORD, (LPARAM)RGB(0,0,255));
	editCtrl->SendMessage(SCI_STYLESETFORE, SCE_P_STRING, (LPARAM)RGB(226,31,31));
	editCtrl->SendMessage(SCI_STYLESETFORE, SCE_P_COMMENTLINE, (LPARAM)RGB(0,100,0));
	editCtrl->SendMessage(SCI_STYLESETFORE, SCE_P_COMMENTBLOCK, (LPARAM)RGB(0,100,0));
	editCtrl->SendMessage(SCI_SETINDENTATIONGUIDES, SC_IV_LOOKBOTH, 0);

	// タブ幅を4スペース分に設定
	editCtrl->SendMessage(SCI_SETTABWIDTH, 4, 0);

	// マージン0を行番号表示用に設定
	editCtrl->SendMessage(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);

	// 行番号の幅（ピクセル単位）を設定
	editCtrl->SendMessage(SCI_SETMARGINWIDTHN, 0, 40);

	// 行番号の文字色を設定（例：グレー）
	editCtrl->SendMessage(SCI_STYLESETFORE, STYLE_LINENUMBER, RGB(128, 128, 128));

	// 行番号の背景色を設定（例：白）
	editCtrl->SendMessage(SCI_STYLESETBACK, STYLE_LINENUMBER, RGB(255, 255, 255));

	// 水平スクロールバーを非表示にする
	editCtrl->SendMessage(SCI_SETHSCROLLBAR, FALSE, 0);
	// ワードラップを有効にする
	editCtrl->SendMessage(SCI_SETWRAPMODE, SC_WRAP_CHAR, 0);

	// フォント名とサイズを設定
	editCtrl->SendMessage(SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Consolas");
	editCtrl->SendMessage(SCI_STYLESETSIZE, STYLE_DEFAULT, 12);

	// タブ文字を可視化
	editCtrl->SendMessage(SCI_SETVIEWWS, SCWS_VISIBLEONLYININDENT, 0);
	editCtrl->SendMessage(SCI_SETWHITESPACEFORE, TRUE, RGB(192, 192, 192)); // 前景色

	//editCtrl->StyleClearAll();

	SetIcon(IconLoader::Get()->LoadDefaultIcon(), FALSE);


	UpdateStatus();
	UpdateDataWrapper(FALSE);

	mIsTested = true;
	UpdateTitle();

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
		mIsError = true;
	}

	UpdateTitle();
	GetDlgItem(IDOK)->EnableWindow(isValid && mIsTested);
	return isValid;
}

void CommandEditDialog::OnUpdateStatus()
{
	UpdateDataWrapper();
	UpdateStatus();
	UpdateDataWrapper(FALSE);
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
	UpdateDataWrapper();
	if (TestSyntax() == false) {
		UpdateStatus();
		UpdateDataWrapper(FALSE);
		return ;
	}
	if (UpdateStatus() == false) {
		return ;
	}

	__super::OnOK();
}

void CommandEditDialog::OnButtonHotKey()
{
	UpdateDataWrapper();

	if (CommandHotKeyDialog::ShowDialog(mParam.mName, mParam.mHotKeyAttr, this) == false) {
		return ;
	}
	UpdateStatus();
	UpdateDataWrapper(FALSE);
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

BOOL CommandEditDialog::UpdateDataWrapper(BOOL bSaveAndValidate)
{
	BOOL result = UpdateData(bSaveAndValidate);

	auto editCtrl = GetDlgItem(IDC_EDIT_SCRIPT);

	if (bSaveAndValidate) {
		// テキストを取得
		int length = (int)editCtrl->SendMessage(SCI_GETTEXTLENGTH, 0, 0);
		std::string buffer(length + 1, '\0');

		editCtrl->SendMessage(SCI_GETTEXT, length + 1, (LPARAM)buffer.data());
		UTF2UTF(buffer, mParam.mScript);
	}
	else {
		// テキストを設定
		std::string scriptA;
		UTF2UTF(mParam.mScript, scriptA);
		editCtrl->SendMessage(SCI_SETTEXT, 0, (LPARAM)(LPCSTR)scriptA.c_str());
	}

	return result;
}

void CommandEditDialog::OnButtonSyntaxCheck()
{
	UpdateDataWrapper();

	TestSyntax();
	UpdateStatus();

	UpdateDataWrapper(FALSE);
}

void CommandEditDialog::OnButtonRun()
{
	UpdateDataWrapper();

	if (TestSyntax() == false) {
		UpdateStatus();
		UpdateDataWrapper(FALSE);
		return;
	}

	auto loader = PythonDLLLoader::Get();
	auto proxy = loader->GetLibrary();


	std::string src;
	UTF2UTF(mParam.mScript, src);
	char* errMsg= nullptr;
	CWaitCursor wc;
	if (proxy->Evaluate(src.c_str(), nullptr, &errMsg) == false) {
		UTF2UTF(errMsg, mResultMsg);
		proxy->ReleaseBuffer(errMsg);
	}
	else {
		mResultMsg.Empty();
		mIsError = false;
	}

	UpdateDataWrapper(FALSE);
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

