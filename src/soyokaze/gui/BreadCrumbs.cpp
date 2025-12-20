#include "pch.h"
#include "BreadCrumbs.h"
#include <deque>
#include <tuple>

BreadCrumbs::BreadCrumbs(CTreeCtrl* treeCtrl, HTREEITEM treeItem)
{
	ASSERT(treeCtrl);

	ItemList tmp;

	HTREEITEM h = treeItem;
	while(h) {
		CString str = treeCtrl->GetItemText(h);
		h = treeCtrl->GetParentItem(h);

		tmp.push_back(str);
	}

	std::reverse(tmp.begin(), tmp.end());
	mItems.swap(tmp);
}

BreadCrumbs::BreadCrumbs(CString str)
{
	ItemList tmp;

	int n = 0;
	CString token = str.Tokenize(_T(">"), n);
	while(token.IsEmpty() == FALSE) {
		token.Trim();
		tmp.push_back(token);

		token = str.Tokenize(_T(">"), n);
	}

	mItems.swap(tmp);
}


BreadCrumbs::~BreadCrumbs()
{
}

CString BreadCrumbs::ToString() const
{
	CString str;

	for (auto& item : mItems) {
		if (str.IsEmpty() == FALSE) {
			str += _T(" > ");
		}
		str += item;
	}

	return str;
}

int BreadCrumbs::GetDepth() const
{
	return (int)mItems.size();
}

CString BreadCrumbs::GetItem(int depth) const
{
	ASSERT(0 <= depth && depth < (int)mItems.size());
	return mItems[depth];
}

static bool 
IsMatch(CTreeCtrl* treeCtrl, HTREEITEM hItem, const CString& item)
{
	ASSERT(treeCtrl);
	return item == treeCtrl->GetItemText(hItem);
}

HTREEITEM BreadCrumbs::FindTreeItem(CTreeCtrl* treeCtrl) const
{
	ASSERT(treeCtrl);

	using Iterator = ItemList::const_iterator;
	std::deque<std::tuple<HTREEITEM, Iterator> > stk;

	HTREEITEM h = treeCtrl->GetRootItem();
	while(h) {
		stk.push_back({h, mItems.begin()});
		h = treeCtrl->GetNextSiblingItem(h);
	}

	while (stk.empty() == false) {

		auto elem = stk.front();
		stk.pop_front();

		h = std::get<0>(elem);
		Iterator itCurrentPos = std::get<1>(elem);
		if  (IsMatch(treeCtrl, h, *itCurrentPos) == false) {
			continue;
		}
		if (itCurrentPos + 1 == mItems.end()) {
			// 完全一致するものが見つかったので、末尾のHTREEITEMを返す
			return h;
		}

		// 子要素を検索対象として積む
		h = treeCtrl->GetChildItem(h);
		while (h) {
			stk.push_back({h, itCurrentPos + 1});
			h = treeCtrl->GetNextSiblingItem(h);
		}
	}

	return nullptr;
}

