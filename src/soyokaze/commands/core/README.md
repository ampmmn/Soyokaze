# commands/core

コマンド関連機能を実現するためのメインとなる機能を置く

## コマンドの拡張インターフェース

`Command`はコマンド名、説明、実行アクションなどの基本機能を定義する。追加機能はすべてのコマンドに持たせず、必要なコマンドが`UnknownIF::QueryInterface()`を通じて個別のインターフェースを公開する。

|インターフェース|役割|
|---|---|
|[`SelectionBehavior`](./SelectionBehavior.h)|候補の選択・解除時の処理、入力欄の補完、コマンド実行後にメインウインドウを閉じる方針を提供する|
|[`PreviewProvider`](./PreviewProvider.h)|選択中コマンドのプレビュー表示に使う`PreviewWindow`を提供する。プレビューが不要な場合は`nullptr`を返す|

`SelectionBehavior::GetCloseWindowPolicy()`は`CLOSEWINDOW_ASYNC`、`CLOSEWINDOW_SYNC`、`CLOSEWINDOW_NOCLOSE`のいずれかを返し、実行後のウインドウ処理を選択する。`PreviewProvider::FetchPreviewWindow()`は、メインウインドウ内のプレビュー領域へ接続するビューを返す。プレビュー領域の配置と表示は`features/main`側が担当する。


## コマンド編集の流れ

```plantuml
participant "CommandRepository" as repos
participant "Command" as cmd
participant "CommandEditor" as editor
participant "CommandDialog" as dlg

-> repos : コマンド編集要求
activate repos

note over repos : コマンドが編集可能かを問い合わせる
repos -> cmd ++ : IsEdiable()
return true

alt true


    note over repos : 編集用のダイアログを取得する
	repos -> cmd ++ : CreateEditor()
		cmd -> editor ++ : create
            editor -> dlg ++ : create
            return
		return

	return dlg

    note over repos : 編集用のダイアログを表示する
	repos -> editor ++ : DoModal()
        editor -> dlg ++ : DoModal()
        return OK or Cancel
	return OK or Cancel
	
    note over repos : OKで閉じられたら元のコマンドに設定の変更結果を反映する
	alt OK
		repos -> cmd ++ : Apply(editor)
			cmd -> editor ++ : getSettings()
			return settings

			cmd -> cmd : apply setttings

		return

        note over repos : 変更後のコマンドを再登録する
        repos -> repos : reregister(cmd)

	end

    note over repos : ダイアログを破棄
    repos -> editor ++ : Release()
        editor -> dlg ++ : delete
            destroy dlg
        return
        destroy editor
    return

end

deactivate repos
<-- repos

```
