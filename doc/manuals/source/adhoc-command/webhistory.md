# Browser History Command

Webブラウザの閲覧履歴を検索するためのコマンド。  
検索ワードを含むタイトルやURLの履歴を候補として表示できる。  

![](../image/adhoc-command/webhistory/sample-without-prefix.png)

該当する項目を選択して実行すると、履歴のページをブラウザで開く。

Edge/Chromeに対応している。  

## Usage

- 入力欄にキーワードを入力すると、そのキーワードをタイトルに含むWeb履歴を表示する
- アプリケーション設定でプレフィックスが設定されている場合、そのプレフィックスを入力すると機能が発動する
  - 以下はプレフィックスを`h`としている場合の例
![](../image/adhoc-command/webhistory/sample-with-prefix.png)
- アプリケーション設定で`検索を実行する最小文字数`を設定している場合、その文字数以上のキーワードを入力したときに機能が発動する

## Settings

機能は初期状態で有効になっている。
機能を使わない場合は、アプリケーション設定の `拡張機能` > `Web履歴検索` から機能を無効化することができる。  

![](../image/adhoc-command/webhistory/appsetting.png)


## Restrictions

- タイムアウトについて  
履歴を探すためのQuery処理の際、指定した時間が経過したら処理を打ち切る。(150msec固定)  
打ち切るまでに見つかった候補を表示する。  

## Per-Key Behavior

|押下キー|動作|
|--|--|
|`Enter`|該当するページをブラウザで開く|
|`Shift-Enter`|URLをクリップボードにコピーする|

