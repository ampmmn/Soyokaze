# Everything

[Everything](https://www.voidtools.com/)の検索機能を利用して、フォルダやファイルを検索する。

![](../image/adhoc-command/everything-introduction.png)

## Usage

- 実行環境にEverythingがインストール済で、実行中であること
  - Everything Liteは不可
- キーワードを入力すると、Everythingでの検索結果を候補として表示する
- Everythingが実行されている状態では自動で機能が発動する
  - アプリケーション設定でプレフィックスを設定すると、そのプレフィックスを入力したときだけ機能が発動するようになる

## Settings

初期状態は機能は有効になっている。
機能を使用しない場合はアプリケーション設定の `拡張機能` > `Everything` から無効化できる。

![](../image/adhoc-command/everything-appsetting.png)

## Restrictions

- 一度に表示する候補件数の上限は32件
- ファイル更新日時が新しい順に候補を表示する
- この機能ではMigemo検索は適用されない

