# Visual Studio Code履歴

[Visual Studio Code](https://code.visualstudio.com/)で最近開いた項目(ワークスペース/フォルダ/ファイル)を候補として表示する。  
実行すると、該当する項目をVisual Studio Codeで開く。

![](../image/adhoc-command/vscode/introduction.png)


## Usage

- プレフィックス(初期値は`vscode`)を入力すると、Visual Studio Codeの履歴一覧を候補として表示する
- 該当する候補を選択して実行すると、対応する項目をVisual Studio Codeで開く
- この機能を利用するためには、実行環境にVisual Studio Codeがインストールされている必要がある
- この機能が扱えるのはVisual Studio Codeの標準機能で利用できる`ワークスペース` `フォルダ` `ファイル` のみ
  - 拡張機能などで追加で扱えるようになった要素は対象外  
(そういうものがあるかはわからないが)

## Settings

機能は初期状態で有効になっている。
機能を使わない場合は、アプリケーション設定の `拡張機能` > `Visual Studio Code` から機能を無効化することができる。  

![](../image/adhoc-command/vscode/appsetting.png)

## Per-Key Behavior

|押下キー|動作|
|--|--|
|`Enter`|VSCode上で項目を開く<br>(既存のウインドウがある場合はそれを使って開く)|
|`Shift-Enter`|新しいVSCodeのウインドウを作成して項目を開く|
|`Ctrl-Enter`|パスをクリップボードにコピーする|


