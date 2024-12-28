# simple_dict

Excelデータを辞書データとして検索するためのコマンド

## クラス

```plantuml

hide circle

class CommandRepository

class SimpleDictAdhocCommand
class SimpleDictCommand
class SimpleDictCommandUpdateListenerIF
class SimpleDictDatabase
class SimpleDictEditDialog
class SimpleDictParam
class SimpleDictProvider

SimpleDictDatabase -up-|> SimpleDictCommandUpdateListenerIF
SimpleDictProvider -le> CommandRepository : 生成したコマンドを登録
SimpleDictProvider -down-> SimpleDictCommand : 生成
SimpleDictCommand -> SimpleDictEditDialog : 設定ダイアログ表示
SimpleDictCommand o.. SimpleDictParam
SimpleDictProvider "1" o..> "1" SimpleDictDatabase
SimpleDictEditDialog ..> SimpleDictParam : 設定
SimpleDictProvider ..> SimpleDictAdhocCommand : 生成
```

### SimpleDictProvider

`SimpleDictCommand`を `CommandRepository`に登録するためのプロバイダークラス


### SimpleDictCommand

- 1つのコマンドにつき1つの辞書データを持つ

### SimpleDictParam

`SimpleDictCommand`の属性を集めたデータクラス

### SimpleDictEditDialog

コマンドの新規作成/編集ダイアログを実装したクラス

class SimpleDictAdhocCommand

### SimpleDictCommandUpdateListenerIF

`SimpleDictCommand`クラスの変更を受け取るためのリスナークラス。  
`SimpleDictDatabase`がこのリスナー。

### SimpleDictDatabase

`SimpleDictCommand`が持つ情報(ファイル/シート/セル範囲)に基づいて、Excelファイルからのデータ読み込みを行い、
ばらした辞書データを保持するクラス。

シングルトン。コマンド毎の辞書情報をmapで管理する  
スレッドを内部で作成し、スレッド側で一定間隔でコマンド設定に更新があったかどうかを調べる。
更新があったら、辞書データの再生成を行う。

現在のところ、ファイルの更新監視は未実装なので、元ファイルを更新しても、辞書データ側への反映は行われない。  
(プログラムを再起動すれば、元ファイルからの読み込みが再度行われるため、元データの変更も反映される。)

そんな頻繁に更新されるものをデータソースとすることを想定してないので、ここは割り切り。

SimpleDictCommand側のコマンドの設定変更を`SimpleDictCommandUpdateListenerIF`経由でリスナー通知で受け取る。  
それを契機に、辞書データの再生成を行う

### SimpleDictAdhocCommand

検索結果を候補欄に表示するために生成されるインスタンス  
入力欄に入力されたテキストにマッチする辞書データの要素1つにつき、1つのSimpleDictAdhocCommandインスタンスを生成する。


## シーケンス

未作成

## メモ

- Excelの機能をCOM経由で利用している
  - このため、実行環境にExcelがインストールされている必要あり。

## 既知の問題

- 入力欄にコマンド名を入力した後、2つ以降のキーワードで候補の絞り込みを行うことになるが、現状のつくりとして、Migemo検索が有効なのは1つめのキーワードのみとしている
  - 1つ目のキーワードにコマンド名がくるため、このコマンドの候補絞り込みではMigemoは機能しない。

