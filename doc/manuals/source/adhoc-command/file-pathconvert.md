# fileプロトコルをローカルパスに変換

`file://...`で始まるパスをローカルパスやUNC形式のパスに変換し、変換したパスに対して下記の操作を行うことができる。

- fileプロトコルの例  
`file:///C:/Windows/WindowsUpdate.log` → `C:\Windows\WindowsUpdate.log`

## 実行時の動作

|押下キー|動作|
|--|--|
|`Enter`|変換後のパスをクリップボードにコピー|
|`Shift-Enter`|開く(ファイルを実行する)|
|`Ctrl-Enter`|パスを開く(パスが存在するフォルダをファイラーで開く)|

