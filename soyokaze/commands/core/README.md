# commands/core

コマンド関連機能を実現するためのメインとなる機能を置く


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
