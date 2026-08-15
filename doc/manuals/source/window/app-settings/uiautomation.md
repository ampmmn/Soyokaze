(extension-uiautomation)=
# Extension > UIAutomation

![](../../image/window/app-settings/uiautomation/uiautomation.png)

- `UI要素の検索機能を有効にする`  
[UI要素](/adhoc-command/uiautomation)機能をオンにする。  
アプリ内のボタンをメニューなどを直接検索・操作できるようにする機能。
- `プレフィックス`  
UI要素検索を開始するためのキーワードを指定する。  
空欄の場合は、UI要素検索が常に有効になる。
- `Win32アプリケーションのメニュー検索を有効にする`  
Win32アプリケーションのウインドウ上部にあるメニュー項目を候補として表示する。  
  - `全てのウインドウのメニュー検索を有効にする`  
画面上に表示されているすべてのWin32アプリケーションのメニュー項目を候補に含める。
オフの場合は、アクティブなウインドウのみを対象とする。
- `デバッグ出力を有効にする`  
デバッグ用のコマンド `uiautomation-debug-dump`を使用可能にする。
このコマンドを実行すると、ログ出力ディレクトリに `uiautomation.json`を生成し、UIAutomationElementの階層構造をJSON形式で保存する。
