(extension-everything)=
# Extension > Everything

![](../../image/adhoc-command/everything-appsetting.png)

- `機能を有効にする`  
[Everything検索](/adhoc-command/everything)機能が有効にする。
- `プレフィックス`  
検索を開始するためのキーワードを指定する。  
空欄にすると、Everything検索が常に有効になる。
- `Everythingが起動していなかった場合に起動する`  
Everythingのアプリが起動していなかった場合に起動を試みる。  
(Everything検索を行うためにはEverythingのアプリが起動している必要がある)
- `Everything.exeのパス`   
アプリを起動する際の実行ファイルのパスを指定する。
- `検索を実行する最小文字数`  
その文字数以上のキーワードを入力したときに検索を行う。0を指定すると常に検索を行う。
  - 少ない文字数で検索を実行したときに以下の問題がある。これを回避するための設定
    - たいていの場合ヒット件数が多くなる。結果、他の候補が隠れてしまう
    - 一方で所望の結果が得られる可能性は低い
