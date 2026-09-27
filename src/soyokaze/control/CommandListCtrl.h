#pragma once

#include <memory>
#include <vector>

namespace launcherapp {
namespace core {
class Command;
}
}

class CommandHotKeyMappings;

/**
  コマンド一覧を表示するリストコントロール
*/
class CommandListCtrl : public CListCtrl
{
public:
	enum class ColumnMode {
		CommandInfo,
		WithHotKey,
	};

	enum class SelectionMode {
		Single,
		Multiple,
	};

	CommandListCtrl();
	virtual ~CommandListCtrl();

	/**
	  表示する列の構成を設定する
	  @param[in] mode 列構成
	*/
	void SetColumnMode(ColumnMode mode);

	/**
	  選択モードを設定する
	  @param[in] mode 単一選択または複数選択
	*/
	void SetSelectionMode(SelectionMode mode);

	/**
	  ホットキー情報の参照先を設定する
	  参照先は本クラスを使用している間有効であること
	  @param[in] mappings ホットキー情報
	*/
	void SetHotKeyMappings(const CommandHotKeyMappings* mappings);

	/**
	  ダイアログに配置されたリストコントロールを初期化する
	*/
	void Initialize();

	/**
	  表示対象のコマンドを設定する
	  コマンドの参照を取得し、以前の一覧が保持していた参照を解放する
	  @param[in] commands 表示対象のコマンド
	*/
	void SetCommands(const std::vector<launcherapp::core::Command*>& commands);

	/**
	  フィルター文字列を設定し、表示対象を更新する
	  @param[in] filterText 絞り込み文字列
	*/
	void SetFilterText(const CString& filterText);

	/**
	  現在選択されているコマンドを取得する
	  @return 選択されているコマンド。返されるポインタは本クラスが一覧に保持している間有効
	*/
	std::vector<launcherapp::core::Command*> GetSelectedCommands();

	/**
	  指定したコマンドを選択する
	  @param[in] command 選択対象のコマンド
	  @param[in] isRedrawRequired 再描画が必要な場合はtrue
	*/
	void SelectCommand(launcherapp::core::Command* command, bool isRedrawRequired);

	/**
	  指定したコマンドを選択する
	  @param[in] commands 選択対象のコマンド
	  @param[in] isRedrawRequired 再描画が必要な場合はtrue
	*/
	void SelectCommands(const std::vector<launcherapp::core::Command*>& commands, bool isRedrawRequired);

	/**
	  指定位置のコマンドを選択する
	  @param[in] index 表示一覧上の位置
	  @param[in] isRedrawRequired 再描画が必要な場合はtrue
	*/
	void SelectItemAt(int index, bool isRedrawRequired);

	/**
	  表示一覧上のコマンドを取得する
	  @param[in] index 表示一覧上の位置
	  @return コマンド。範囲外の場合はnullptr
	*/
	launcherapp::core::Command* GetCommandAt(int index) const;

	/**
	  表示一覧の項目数を取得する
	  @return 表示項目数
	*/
	int GetVisibleCommandCount() const;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFindCommand(NMHDR* pNMHDR, LRESULT* pResult);
};
