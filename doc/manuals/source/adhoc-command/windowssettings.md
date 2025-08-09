# Windows Settings

Windowsの設定アプリの各ページ要素を候補として表示し、実行することができる。

## 例: Windows Updateを表示する

![](../image/input-windowsupdate.png)  
↓  
![](../image/windowsupdate.png)

## 注意

- この機能は正式なAPIを用いて一覧やページ名称を動的に取得していものではなく、アプリが独自に一覧やページ名の対応表を静的に保持して、それに従っているものである。  
このため、将来のWindowsの更新による変更に追従できない可能性がある(ページ追加、廃止、ページ名変更など)
- 一部の設定ページは開くために特定の要件を満たす必要があるものがあるが、そのあたりはとくに考慮していない
- ページ一覧は下記URLから取得したもの  
[https://learn.microsoft.com/ja-jp/windows/apps/develop/launch/launch-settings-app](https://learn.microsoft.com/ja-jp/windows/apps/develop/launch/launch-settings-app)

