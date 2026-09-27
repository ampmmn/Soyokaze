#pragma once

class CTreeCtrl;

// パンくずリストクラス
// 設定ダイアログ(SettingDialogBase)で表示しているページ階層を表す
class BreadCrumbs
{
	using ItemList = std::vector<CString>;
public:
	// コンストラクタ
	BreadCrumbs(CTreeCtrl* treectrl, HTREEITEM treeItem);
	// コンストラクタ
	BreadCrumbs(CString str);
	~BreadCrumbs();

	// 階層構造を文字列化する
	CString ToString() const;

	// 階層深さを得る
	int GetDepth() const;
	// 階層ごとのページ名を取得
	CString GetItem(int depth) const;

	// パンくずリストが示す階層に対応するHTREEITEMを検索する
	HTREEITEM FindTreeItem(CTreeCtrl* treeCtrl) const;

private:
	ItemList mItems;
};
