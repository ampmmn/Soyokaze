# WinSCP

WinSCPのサイトマネージャに登録されているセッション一覧を候補として表示する。

![](../image/adhoc-command/winscp-introduction2.png)

![](../image/adhoc-command/winscp-introduction.png)

実行すると、WinScpアプリケーションを起動し、接続を開始する。

## Usage

- WinSCPがインストールされていること
  - ポータブル版の場合、どこに実行ファイルがあるかを検出できないため、アプリケーション設定でパスを指定する必要がある
- WinSCPのサイトマネージャに登録された接続先の情報を候補として表示する
- 発動のためのキーワード(プレフィックス)は無い  
サイトマネージャーに登録した名前を入力すると、該当するセッションが候補に表示される

## Settings

初期状態では無効になっている。

アプリケーション設定の `拡張機能` > `WinSCP` から機能を有効化することができる。  

![](../image/adhoc-command/winscp-appsetting.png)

