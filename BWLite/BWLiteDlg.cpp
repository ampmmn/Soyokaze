
// BWLiteDlg.cpp : 実装ファイル
//

#include "pch.h"
#include "framework.h"
#include "BWLite.h"
#include "BWLiteDlg.h"
#include "CommandMap.h"
#include "afxdialogex.h"
#include "WindowPlacementUtil.h"
#include "SharedHwnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBWLiteDlg ダイアログ

CBWLiteDlg::CBWLiteDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BWLITE_DIALOG, pParent), m_pCommandMap(new CommandMap()), m_pSharedHwnd(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON2);
	m_pCurCommand = nullptr;
}

CBWLiteDlg::~CBWLiteDlg()
{
	delete m_pCommandMap;
	delete m_pSharedHwnd;
}

void CBWLiteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COMMAND, m_strCommand);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, m_strDescription);
	DDX_Control(pDX, IDC_LIST_CANDIDATE, mCandidateListBox);
}

BEGIN_MESSAGE_MAP(CBWLiteDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT_COMMAND, OnEditCommandChanged)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CBWLiteDlg メッセージ ハンドラー

BOOL CBWLiteDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	m_pSharedHwnd = new SharedHwnd(GetSafeHwnd());

	m_strDescription.LoadString(ID_STRING_DEFAULTDESCRIPTION);

	// ウインドウ位置の復元
	util::window::LoadPlacement(this, _T("LauncherWindowPos"));

	// 設定値の読み込み
	m_pCommandMap->Load();

	UpdateData(FALSE);

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CBWLiteDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CBWLiteDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CommandMap* CBWLiteDlg::GetCommandMap()
{
	return m_pCommandMap;
}

void CBWLiteDlg::SetDescription(const CString& msg)
{
	m_strDescription = msg;
	UpdateData(FALSE);
}

void CBWLiteDlg::OnEditCommandChanged()
{
	UpdateData();

	mCandidateListBox.ResetContent();

	if (m_strCommand.IsEmpty()) {
		CString strMisMatch;
		strMisMatch.LoadString(ID_STRING_DEFAULTDESCRIPTION);
		SetDescription(strMisMatch);
		return;
	}

	GetCommandMap()->Query(m_strCommand, mCandidates);
	if (mCandidates.size() > 0) {
		auto pCmd = mCandidates[0];
		SetDescription(pCmd->GetDescription());
		m_pCurCommand = pCmd;

		for (auto& item : mCandidates) {
			mCandidateListBox.AddString(item->GetName());
		}

	}
	else {
		CString strMisMatch;
		strMisMatch.LoadString(ID_STRING_MISMATCH);
		SetDescription(strMisMatch);
		m_pCurCommand = nullptr;
	}
}

void CBWLiteDlg::OnOK()
{
	// ToDo: コマンドを実行する
	if (m_pCurCommand) {
		if (m_pCurCommand->Execute() == FALSE) {
			AfxMessageBox(m_pCurCommand->GetErrorString());
		}
	}

	ShowWindow(SW_HIDE);
}

void CBWLiteDlg::OnCancel()
{
	ShowWindow(SW_HIDE);
}

void CBWLiteDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if (bShow) {
		GetDlgItem(IDC_EDIT_COMMAND)->SetFocus();
	}
	else {
		m_strDescription.LoadString(ID_STRING_DEFAULTDESCRIPTION);
		m_strCommand.Empty();
		m_pCurCommand = nullptr;
		mCandidateListBox.ResetContent();
		UpdateData(FALSE);
		util::window::SavePlacement(this, _T("LauncherWindowPos"));
	}
}
