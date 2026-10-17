# Office Favorites

Word、Excel、PowerPointでお気に入り登録(ピン留めした)ファイルを検索候補として表示する。ローカルに保存されたファイルのほか、SharePointやOneDrive上のファイルも対象になる。

![](../image/adhoc-command/office-favorites/input.png)

候補はアプリごとに「お気に入り(Word)」「お気に入り(Excel)」「お気に入り(PowerPoint)」として表示される。名前を入力して候補を絞り込み、対象のファイルを実行できる。

## Usage

- Officeでピン留めしたファイル名を入力して候補を検索する

![](../image/adhoc-command/office-favorites/example.png)

ファイル名は、取得できる場合はOfficeに保存された表示名を使い、それ以外はファイル名を使う。

## Settings

この機能は初期状態で有効になっている。アプリケーション設定の `拡張機能` > `システム関連の機能` にある `Word/Excel/PowerPointのお気に入りを表示する` で切り替える。

![](../image/adhoc-command/office-favorites/setting.png)

## Notes

- ローカルファイルは、ファイルが存在する項目だけが候補に表示される
- SharePoint／OneDrive上のピン留め項目は、Office側の同期状況によって候補への反映に時間がかかる場合がある

## Per-Key Behavior

|押下キー|動作|
|--|--|
|`Enter`|ファイルを開く|
|`Ctrl-Enter`|ファイルのあるフォルダを開く(ローカルファイルのみ)|
|`Alt-Enter`|ファイルのプロパティを開く(ローカルファイルのみ)|


