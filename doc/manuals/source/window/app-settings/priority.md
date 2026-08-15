(command-execute-priority)=
# Command Execute > Priority

コマンドの優先度を設定することにより、候補欄に表示されるコマンドの順序を制御するための画面。

![](../../image/window/app-settings/priority/setting.png)

- `コマンドの優先度`
  - コマンドごとの優先度が表示されたリスト
  - 優先度の数値が高いものほど先に表示される。
- `フィルター`
  - リスト上に表示するコマンドの絞り込みを行うことができる
- `優先度変更`
  - `コマンドの優先度`リストで選択した項目の優先度を変更する
- `すべてリセット`
  - `コマンドの優先度`リスト上に表示されたすべての項目の優先度を0にする

(command-execute-priority-example)=
### Example

たとえば、下記の設定がされていた場合、
入力欄に「b」と打った場合に、`bookmark` `blog-hatena` `board` の順に表示される。  

|コマンド|優先度|
|--|--|
|blog-hatena|5|
|board|0|
|bookmark|10|


(command-execute-priority-restrictions)=
### Restrictions

- 利用者が[登録したコマンド](/usage/how_to_use.md#register-command)に対してのみ、コマンドの優先度を設定することができる
  - [コマンドとして登録していなくても実行できる機能](/adhoc-commands) に由来するコマンドに対して優先度を設定することはできない

