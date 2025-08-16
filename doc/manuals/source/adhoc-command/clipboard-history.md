# Clipboard History

クリップボードに保存されたテキストの履歴を候補として表示する機能。  
候補を選択すると、それをクリップボードに再コピーしたり、コピペしたりなどすることができる。

![](../image/clipboardhistory-prefix.png)

## Usage

- プレフィックスとなる文字列を入力すると、クリップボードに保存されたテキストが候補として表示される。
(上記の例だと、`cb`がプレフィックス)

- 初期状態では本機能は無効化されている。利用するには、アプリ設定から有効化する必要がある。


## Setting

各種設定は[アプリ設定](/window/app-settings.md#extensions-clipboard-history)から設定することができる。


## Preview

アプリ設定でプレビューウインドウの機能を有効にしている場合、
候補欄で選択したクリップボード履歴のテキスト内容を表示するウインドウが表示される。

![](../image/adhoc-command/clipboardhistory-preview.png)
