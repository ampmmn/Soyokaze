#include "pch.h"
#include "control/CommandListCtrl.h"
#include "commands/core/CommandIF.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "matcher/PartialMatchPattern.h"
#include "resource.h"
#include "utility/RefPtr.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
	enum SortType {
		SORT_ASCEND_NAME,
		SORT_DESCEND_NAME,
		SORT_ASCEND_DESCRIPTION,
		SORT_DESCEND_DESCRIPTION,
		SORT_ASCEND_CMDTYPE,
		SORT_DESCEND_CMDTYPE,
		SORT_ASCEND_HOTKEY,
		SORT_DESCEND_HOTKEY,
	};

	enum ColumnIndex {
		COL_CMDNAME,
		COL_CMDTYPE,
		COL_DESCRIPTION,
		COL_HOTKEY,
	};
}

struct CommandListCtrl::PImpl
{
	CommandListCtrl* mOwner{nullptr};

	void SortCommands();
	void UpdateVisibleCommands(const std::vector<RefPtr<launcherapp::core::Command>>& selectedCommands, bool isRedrawRequired);
	void SelectCommands(const std::vector<launcherapp::core::Command*>& commands, bool isRedrawRequired);
	std::vector<RefPtr<launcherapp::core::Command>> GetSelectedCommandRefs();

	ColumnMode mColumnMode{ColumnMode::CommandInfo};
	SelectionMode mSelectionMode{SelectionMode::Single};
	const CommandHotKeyMappings* mHotKeyMappings{nullptr};
	std::vector<launcherapp::core::Command*> mCommands;
	std::vector<launcherapp::core::Command*> mVisibleCommands;
	CString mFilterText;
	int mSortType{SORT_ASCEND_NAME};
	bool mIsInitialized{false};
};
/**
  現在のソート条件に従ってコマンドを並べ替える
*/
void CommandListCtrl::PImpl::SortCommands()
{
	if (mSortType == SORT_ASCEND_NAME) {
		std::sort(mCommands.begin(), mCommands.end(), [](auto left, auto right) {
			return left->GetName().CompareNoCase(right->GetName()) < 0;
		});
	}
	else if (mSortType == SORT_DESCEND_NAME) {
		std::sort(mCommands.begin(), mCommands.end(), [](auto left, auto right) {
			return right->GetName().CompareNoCase(left->GetName()) < 0;
		});
	}
	else if (mSortType == SORT_ASCEND_DESCRIPTION) {
		std::sort(mCommands.begin(), mCommands.end(), [](auto left, auto right) {
			return left->GetDescription() < right->GetDescription();
		});
	}
	else if (mSortType == SORT_DESCEND_DESCRIPTION) {
		std::sort(mCommands.begin(), mCommands.end(), [](auto left, auto right) {
			return right->GetDescription() < left->GetDescription();
		});
	}
	else if (mSortType == SORT_ASCEND_CMDTYPE) {
		std::sort(mCommands.begin(), mCommands.end(), [](auto left, auto right) {
			return left->GetTypeDisplayName() < right->GetTypeDisplayName();
		});
	}
	else if (mSortType == SORT_DESCEND_CMDTYPE) {
		std::sort(mCommands.begin(), mCommands.end(), [](auto left, auto right) {
			return right->GetTypeDisplayName() < left->GetTypeDisplayName();
		});
	}
	else if (mSortType == SORT_ASCEND_HOTKEY && mHotKeyMappings != nullptr) {
		std::sort(mCommands.begin(), mCommands.end(), [&](auto left, auto right) {
			return mHotKeyMappings->FindKeyMappingString(left->GetName()) < mHotKeyMappings->FindKeyMappingString(right->GetName());
		});
	}
	else if (mSortType == SORT_DESCEND_HOTKEY && mHotKeyMappings != nullptr) {
		std::sort(mCommands.begin(), mCommands.end(), [&](auto left, auto right) {
			return mHotKeyMappings->FindKeyMappingString(right->GetName()) < mHotKeyMappings->FindKeyMappingString(left->GetName());
		});
	}
}

/**
  現在選択中のコマンドを参照カウント付きで取得する
  @return 選択中コマンドへの参照
*/
std::vector<RefPtr<launcherapp::core::Command>> CommandListCtrl::PImpl::GetSelectedCommandRefs()
{
	std::vector<RefPtr<launcherapp::core::Command>> selectedCommands;
	if (mIsInitialized == false || mOwner->GetSafeHwnd() == nullptr) {
		return selectedCommands;
	}
	for (auto command : mOwner->GetSelectedCommands()) {
		selectedCommands.emplace_back(command, true);
	}
	return selectedCommands;
}

/**
  フィルター文字列に応じて表示一覧と選択状態を更新する
  @param[in] selectedCommands 更新前に選択されていたコマンド
  @param[in] isRedrawRequired 再描画が必要な場合はtrue
*/
void CommandListCtrl::PImpl::UpdateVisibleCommands(
	const std::vector<RefPtr<launcherapp::core::Command>>& selectedCommands,
	bool isRedrawRequired
)
{
	if (mIsInitialized && mOwner->GetSafeHwnd() != nullptr) {
		SelectCommands({}, false);
	}

	mVisibleCommands.clear();
	if (mFilterText.IsEmpty()) {
		mVisibleCommands = mCommands;
	}
	else {
		RefPtr<Pattern> pattern(PartialMatchPattern::Create());
		pattern->SetWholeText(mFilterText);
		for (auto command : mCommands) {
			if (pattern->Match(command->GetName()) == Pattern::Mismatch &&
			    pattern->Match(command->GetDescription()) == Pattern::Mismatch &&
			    pattern->Match(command->GetTypeDisplayName()) == Pattern::Mismatch) {
				continue;
			}
			mVisibleCommands.push_back(command);
		}
	}

	if (mIsInitialized && mOwner->GetSafeHwnd() != nullptr) {
		mOwner->SetItemCountEx(0);
		mOwner->SetItemCountEx((int)mVisibleCommands.size());
		std::vector<launcherapp::core::Command*> selected;
		for (auto& command : selectedCommands) {
			selected.push_back(const_cast<launcherapp::core::Command*>(command.get()));
		}
		SelectCommands(selected, isRedrawRequired);
	}
}

/**
  指定されたコマンドを表示一覧上で選択する
  @param[in] commands 選択対象のコマンド
  @param[in] isRedrawRequired 再描画が必要な場合はtrue
*/
void CommandListCtrl::PImpl::SelectCommands(
	const std::vector<launcherapp::core::Command*>& commands,
	bool isRedrawRequired
)
{
	if (mIsInitialized == false) {
		return;
	}

	int firstSelectedIndex = -1;
	for (int index = 0; index < (int)mVisibleCommands.size(); ++index) {
		auto command = mVisibleCommands[index];
		bool isSelected = std::find(commands.begin(), commands.end(), command) != commands.end();
		if (mSelectionMode == SelectionMode::Single && firstSelectedIndex != -1) {
			isSelected = false;
		}
		bool isFocused = isSelected && firstSelectedIndex == -1;
		if (isSelected && firstSelectedIndex == -1) {
			firstSelectedIndex = index;
		}
		UINT state = isSelected ? LVIS_SELECTED : 0;
		if (isFocused) {
			state |= LVIS_FOCUSED;
		}
		mOwner->SetItemState(index, state, LVIS_SELECTED | LVIS_FOCUSED);
	}
	if (firstSelectedIndex != -1) {
		mOwner->EnsureVisible(firstSelectedIndex, FALSE);
	}
	if (isRedrawRequired) {
		mOwner->Invalidate();
	}
}

CommandListCtrl::CommandListCtrl() : in(std::make_unique<PImpl>())
{
	in->mOwner = this;
}

CommandListCtrl::~CommandListCtrl()
{
	for (auto command : in->mCommands) {
		command->Release();
	}
}

BEGIN_MESSAGE_MAP(CommandListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnHeaderClicked)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_ODFINDITEM, OnFindCommand)
END_MESSAGE_MAP()

void CommandListCtrl::SetColumnMode(ColumnMode mode)
{
	if (in->mColumnMode == mode) {
		return;
	}
	in->mColumnMode = mode;
	if (in->mIsInitialized) {
		Initialize();
	}
}

void CommandListCtrl::SetSelectionMode(SelectionMode mode)
{
	if (in->mSelectionMode == mode) {
		return;
	}
	auto selectedCommands = GetSelectedCommands();
	in->mSelectionMode = mode;
	if (in->mIsInitialized) {
		ModifyStyle(mode == SelectionMode::Multiple ? LVS_SINGLESEL : 0,
		            mode == SelectionMode::Multiple ? 0 : LVS_SINGLESEL);
		if (mode == SelectionMode::Single && selectedCommands.size() > 1) {
			SelectCommand(selectedCommands.front(), false);
		}
	}
}

void CommandListCtrl::SetHotKeyMappings(const CommandHotKeyMappings* mappings)
{
	auto selectedCommands = in->GetSelectedCommandRefs();
	in->mHotKeyMappings = mappings;
	if (in->mSortType == SORT_ASCEND_HOTKEY || in->mSortType == SORT_DESCEND_HOTKEY) {
		in->SortCommands();
		in->UpdateVisibleCommands(selectedCommands, true);
	}
}

void CommandListCtrl::Initialize()
{
	if (GetSafeHwnd() == nullptr) {
		return;
	}

	ModifyStyle(0, LVS_OWNERDATA | LVS_REPORT);
	ModifyStyle(in->mSelectionMode == SelectionMode::Multiple ? LVS_SINGLESEL : 0,
	            in->mSelectionMode == SelectionMode::Multiple ? 0 : LVS_SINGLESEL);
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	if (GetHeaderCtrl() != nullptr) {
		int columnCount = GetHeaderCtrl()->GetItemCount();
		for (int index = 0; index < columnCount; ++index) {
			DeleteColumn(0);
		}
	}

	LVCOLUMN column;
	memset(&column, 0, sizeof(column));
	column.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
	column.fmt = LVCFMT_LEFT;

	CString header;
	header.LoadString(IDS_NAME);
	column.pszText = const_cast<LPTSTR>((LPCTSTR)header);
	column.cx = 100;
	InsertColumn(COL_CMDNAME, &column);

	header.LoadString(IDS_COMMANDTYPE);
	column.pszText = const_cast<LPTSTR>((LPCTSTR)header);
	column.cx = 100;
	InsertColumn(COL_CMDTYPE, &column);

	header.LoadString(IDS_DESCRIPTION);
	column.pszText = const_cast<LPTSTR>((LPCTSTR)header);
	column.cx = 150;
	InsertColumn(COL_DESCRIPTION, &column);

	if (in->mColumnMode == ColumnMode::WithHotKey) {
		header = _T("ホットキー");
		column.pszText = const_cast<LPTSTR>((LPCTSTR)header);
		column.cx = 100;
		InsertColumn(COL_HOTKEY, &column);
	}

	in->mIsInitialized = true;
	SetItemCountEx((int)in->mVisibleCommands.size());
}

void CommandListCtrl::SetCommands(const std::vector<launcherapp::core::Command*>& commands)
{
	auto selectedCommands = in->GetSelectedCommandRefs();

	std::vector<launcherapp::core::Command*> newCommands;
	newCommands.reserve(commands.size());
	for (auto command : commands) {
		if (command == nullptr) {
			continue;
		}
		command->AddRef();
		newCommands.push_back(command);
	}

	if (in->mIsInitialized) {
		in->SelectCommands({}, false);
		SetItemCountEx(0);
	}
	in->mVisibleCommands.clear();
	for (auto command : in->mCommands) {
		command->Release();
	}
	in->mCommands.swap(newCommands);

	in->SortCommands();
	in->UpdateVisibleCommands(selectedCommands, true);
}

void CommandListCtrl::SetFilterText(const CString& filterText)
{
	auto selectedCommands = in->GetSelectedCommandRefs();
	in->mFilterText = filterText;
	in->UpdateVisibleCommands(selectedCommands, true);
}

std::vector<launcherapp::core::Command*> CommandListCtrl::GetSelectedCommands()
{
	std::vector<launcherapp::core::Command*> commands;
	if (in->mIsInitialized == false || GetSafeHwnd() == nullptr) {
		return commands;
	}

	POSITION position = GetFirstSelectedItemPosition();
	while (position != nullptr) {
		int index = GetNextSelectedItem(position);
		if (0 <= index && index < (int)in->mVisibleCommands.size()) {
			commands.push_back(in->mVisibleCommands[index]);
		}
	}
	return commands;
}

void CommandListCtrl::SelectCommand(launcherapp::core::Command* command, bool isRedrawRequired)
{
	std::vector<launcherapp::core::Command*> commands;
	if (command != nullptr) {
		commands.push_back(command);
	}
	in->SelectCommands(commands, isRedrawRequired);
}

void CommandListCtrl::SelectCommands(const std::vector<launcherapp::core::Command*>& commands, bool isRedrawRequired)
{
	in->SelectCommands(commands, isRedrawRequired);
}

void CommandListCtrl::SelectItemAt(int index, bool isRedrawRequired)
{
	SelectCommand(GetCommandAt(index), isRedrawRequired);
}

launcherapp::core::Command* CommandListCtrl::GetCommandAt(int index) const
{
	if (index < 0 || index >= (int)in->mVisibleCommands.size()) {
		return nullptr;
	}
	return in->mVisibleCommands[index];
}

int CommandListCtrl::GetVisibleCommandCount() const
{
	return (int)in->mVisibleCommands.size();
}

void CommandListCtrl::OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	auto notify = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	int clickedColumn = notify->iSubItem;

	int sortType = in->mSortType;
	if (clickedColumn == COL_CMDNAME) {
		in->mSortType = sortType == SORT_ASCEND_NAME ? SORT_DESCEND_NAME : SORT_ASCEND_NAME;
	}
	else if (clickedColumn == COL_CMDTYPE) {
		in->mSortType = sortType == SORT_ASCEND_CMDTYPE ? SORT_DESCEND_CMDTYPE : SORT_ASCEND_CMDTYPE;
	}
	else if (clickedColumn == COL_DESCRIPTION) {
		in->mSortType = sortType == SORT_ASCEND_DESCRIPTION ? SORT_DESCEND_DESCRIPTION : SORT_ASCEND_DESCRIPTION;
	}
	else if (clickedColumn == COL_HOTKEY && in->mColumnMode == ColumnMode::WithHotKey) {
		in->mSortType = sortType == SORT_ASCEND_HOTKEY ? SORT_DESCEND_HOTKEY : SORT_ASCEND_HOTKEY;
	}
	else {
		return;
	}

	auto selectedCommands = in->GetSelectedCommandRefs();
	in->SortCommands();
	in->UpdateVisibleCommands(selectedCommands, true);
}

void CommandListCtrl::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	auto displayInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	auto item = &displayInfo->item;
	if ((item->mask & LVIF_TEXT) == 0) {
		return;
	}

	auto command = GetCommandAt(item->iItem);
	if (command == nullptr) {
		return;
	}

	CString text;
	switch (item->iSubItem) {
	case COL_CMDNAME:
		text = command->GetName();
		break;
	case COL_CMDTYPE:
		text = command->GetTypeDisplayName();
		break;
	case COL_DESCRIPTION:
		text = command->GetDescription();
		break;
	case COL_HOTKEY:
		if (in->mColumnMode == ColumnMode::WithHotKey && in->mHotKeyMappings != nullptr) {
			text = in->mHotKeyMappings->FindKeyMappingString(command->GetName());
		}
		break;
	default:
		return;
	}
	_tcsncpy_s(item->pszText, item->cchTextMax, text, _TRUNCATE);
}

void CommandListCtrl::OnFindCommand(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto findInfo = reinterpret_cast<NMLVFINDITEM*>(pNMHDR);
	*pResult = -1;
	if ((findInfo->lvfi.flags & LVFI_STRING) == 0) {
		return;
	}

	CString searchText = findInfo->lvfi.psz;
	searchText.MakeLower();
	int startPosition = findInfo->iStart;
	int commandCount = (int)in->mVisibleCommands.size();
	if (startPosition < 0 || startPosition >= commandCount) {
		startPosition = 0;
	}

	for (int index = startPosition; index < commandCount; ++index) {
		CString name = in->mVisibleCommands[index]->GetName();
		name.MakeLower();
		if (name.Find(searchText) == 0) {
			*pResult = index;
			return;
		}
	}
	for (int index = 0; index < startPosition; ++index) {
		CString name = in->mVisibleCommands[index]->GetName();
		name.MakeLower();
		if (name.Find(searchText) == 0) {
			*pResult = index;
			return;
		}
	}
}
