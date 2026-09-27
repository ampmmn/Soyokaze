#include "pch.h"
#include "framework.h"
#include "commands/common/CommandSelectDialog.h"
#include "control/CommandListCtrl.h"
#include "control/KeywordEdit.h"
#include "icon/IconLabel.h"
#include "commands/core/CommandRepository.h"
#include "icon/IconLoader.h"
#include "resource.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr UINT TIMERID_FILTER = 1;

namespace launcherapp { namespace commands { namespace common {


using namespace launcherapp::core;

struct CommandSelectDialog::PImpl
{
	CString mName;
	CString mDescription;
	CString mParameter;
	bool mUseParameter{false};

	CString mFilterStr;

	CommandListCtrl mListCtrl;
	std::unique_ptr<IconLabel> mIconLabelPtr;
	KeywordEdit mKeywordEdit;

	// フィルター更新タイマーID
	UINT_PTR mUpdateTimerId{0};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CommandSelectDialog::CommandSelectDialog(CWnd* parent) : 
	CDialogEx(IDD_COMMANDSELECT, parent),
	in(std::make_unique<PImpl>())
{
	//SetHelpPageId("CommandSelect");

	in->mIconLabelPtr = std::make_unique<IconLabel>();
}

CommandSelectDialog::~CommandSelectDialog()
{
}

void CommandSelectDialog::SetCommandName(const CString& name)
{
	in->mName = name;
}

CString CommandSelectDialog::GetCommandName()
{
	return in->mName;
}

void CommandSelectDialog::SetUseParameter(bool useParameter)
{
	in->mUseParameter = useParameter;
}

void CommandSelectDialog::SetParameter(const CString& parameter)
{
	in->mParameter = parameter;
}

CString CommandSelectDialog::GetParameter() const
{
	return in->mParameter;
}


void CommandSelectDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_NAME, in->mName);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, in->mDescription);
	DDX_Text(pDX, IDC_EDIT_FILTER, in->mFilterStr);
	DDX_Text(pDX, IDC_EDIT_PARAM, in->mParameter);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CommandSelectDialog, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_FILTER, OnEditFilterChanged)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_COMMANDS, OnLvnItemChange)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_COMMANDS, OnNMDblclk)
	ON_MESSAGE(WM_APP+1, OnUserMessageKeywrodEditKeyDown)
	ON_MESSAGE(WM_APP+2, OnUserMessageResetContent)
	ON_WM_TIMER()
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL CommandSelectDialog::OnInitDialog()
{
	__super::OnInitDialog();

	SetIcon(IconLoader::Get()->LoadKeywordManagerIcon(), FALSE);

	in->mListCtrl.SubclassDlgItem(IDC_LIST_COMMANDS, this);
	in->mListCtrl.SetColumnMode(CommandListCtrl::ColumnMode::CommandInfo);
	in->mListCtrl.SetSelectionMode(CommandListCtrl::SelectionMode::Single);
	in->mListCtrl.Initialize();
	in->mKeywordEdit.SubclassDlgItem(IDC_EDIT_FILTER, this);
	in->mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);
	in->mIconLabelPtr->DrawIcon(IconLoader::Get()->LoadKeywordManagerIcon());
	GetDlgItem(IDC_STATIC_PARAM)->ShowWindow(in->mUseParameter ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_EDIT_PARAM)->ShowWindow(in->mUseParameter ? SW_SHOW : SW_HIDE);

	// フィルタ欄にプレースホルダーを設定する
	in->mKeywordEdit.SetPlaceHolder(_T("文字列をここに入力するとリストの絞り込みができます"));

	ResetContents();

	// 名前からコマンドを探す
	int commandCount = in->mListCtrl.GetVisibleCommandCount();
	for (int i = 0; i < commandCount; ++i) {

		// コマンド名を小文字に変換したうえで前方一致比較をする
		auto command = in->mListCtrl.GetCommandAt(i);
		CString item = command->GetName();
		if (item.CompareNoCase(in->mName) != 0) {
			continue;
		}
		// コマンドを選択状態にする
		in->mListCtrl.SelectCommand(command, false);
		break;
	}

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

void CommandSelectDialog::OnOK()
{
	UpdateData();
	if (in->mName.IsEmpty()) {
		return;
	}

	__super::OnOK();
}

void CommandSelectDialog::OnCancel()
{
	if (in->mUpdateTimerId != 0) {
		KillTimer(TIMERID_FILTER);
	}
	__super::OnCancel();
}

void CommandSelectDialog::ResetContents()
{
	// コマンド一覧を取得する
	std::vector<Command*> commands;
	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	cmdRepoPtr->EnumCommands(commands);
	in->mListCtrl.SetCommands(commands);
	for (auto command : commands) {
		command->Release();
	}
}

bool CommandSelectDialog::UpdateStatus()
{
	auto selectedCommands = in->mListCtrl.GetSelectedCommands();
	auto selCommand = selectedCommands.empty() ? nullptr : selectedCommands.front();
	bool hasSelection = selCommand != nullptr;

	GetDlgItem(IDOK)->EnableWindow(hasSelection);

	if (selCommand == nullptr) {
		in->mName.Empty();
		in->mDescription.Empty();
		return false;
	}

	CString name = selCommand->GetName();

	in->mIconLabelPtr->DrawIcon(selCommand->GetIcon());
	in->mName = name;
	in->mDescription = selCommand->GetDescription();

	return true;
}

void CommandSelectDialog::UpdateListItems()
{
	in->mListCtrl.SetFilterText(in->mFilterStr);
}

void CommandSelectDialog::OnEditFilterChanged()
{
	// フィルター欄更新のつど更新するのではなく、
	// 最後のキー入力後の0.2秒後に更新を入れる

	if (in->mUpdateTimerId != 0) {
		KillTimer(TIMERID_FILTER);
	}
	in->mUpdateTimerId = SetTimer(TIMERID_FILTER, 200, 0);
}

void CommandSelectDialog::OnTimer(UINT_PTR timerId)
{
	if (timerId != TIMERID_FILTER) {
		return;
	}
	KillTimer(TIMERID_FILTER);
	in->mUpdateTimerId = 0;

	UpdateData();
	UpdateListItems();
	UpdateStatus();
}

/**
 *  リスト欄の要素の状態変更時の処理
 */
void CommandSelectDialog::OnLvnItemChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	*pResult = 0;
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandSelectDialog::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	*pResult = 0;
	OnOK();
}

LRESULT CommandSelectDialog::OnUserMessageKeywrodEditKeyDown(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	// 矢印↓キー押下
	if (wParam ==VK_DOWN) {
		in->mListCtrl.SetFocus();

		if (in->mListCtrl.GetVisibleCommandCount() > 0) {
			in->mListCtrl.SelectItemAt(0, false);
		}
		return 1;
	}
	if (wParam ==VK_UP) {
		in->mListCtrl.SetFocus();

		if (in->mListCtrl.GetVisibleCommandCount() > 0) {
			int visibleItems = in->mListCtrl.GetVisibleCommandCount();
			in->mListCtrl.SelectItemAt(visibleItems - 1, false);
		}
		return 1;
	}
	else if (wParam == VK_TAB) {
		in->mListCtrl.SetFocus();
		return 1;
	}
	return 0;
}

LRESULT CommandSelectDialog::OnUserMessageResetContent(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	ResetContents();
	spdlog::info("KeywordManager content updated.");

	return 0;
}

}}}

