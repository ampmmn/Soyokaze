# Python Extension Command

コマンドを実行時の動作をPythonスクリプトで記述できるコマンド

スクリプトの内容をコマンドの設定としてアプリケーション内に保存しておけるため、スクリプト内容をアプリ内で一元管理できる。

Python 3.12以降が必要

## UseCase

- アプリ独自の機能として、画面操作に関するAPIを提供しており、ちょっとした自動操作の用途で使うことができる
  - 現在はキーボード入力のみ。マウス入力も今後実装予定
- アプリ設定で指定したPython環境でのライブラリのAPIを呼び出すことができるので、それらを利用して様々な処理を実行できる

## Description

![](../image/user-command/py_extension/commandsetting.png)


- `コマンドの名前`  
入力画面からコマンドを実行するためのキーワード
- `説明`  
何のためのコマンドかを記載しておくための説明欄
- `スクリプト` 
実行する処理の内容をPythonスクリプトとして記述する
- `構文チェック`   
`スクリプト`に入力した内容が構文的に正しいかをチェックする  
(チェックして正しいことを確認しないと設定を保存できない)
- `ホットキー`  
コマンドを呼び出すホットキーを定義する


## Remarks

- アプリ設定の`拡張機能`>`Pythonを利用する拡張機能の設定`で設定したPython環境の機能が利用できる
  - その環境に入っているライブラリを利用することが可能

- そのほかにアプリ独自のライブラリを提供する(後述)
  - キー入力やウインドウ関連の機能を提供する
    - もちろん、他のPythonライブラリを使うこともできる

## Restrictions

- Pythonスクリプトを並列で実行することはできない
  - 先行するPython拡張コマンドの実行が完了しないうちに後続のPython拡張コマンドが実行された場合、後続のコマンドは実行されない
(エラーメッセージが表示される)
  - 同様に[簡易電卓機能](/adhoc-command/calclator)も利用できなくなる
  - Python拡張コマンドが実行中かどうかを確認する機能や、実行を止める機能は提供していない

## Library Reference

アプリ独自の機能として、以下のクラスを提供する。  
これらは`import`しなくても利用できる。

### key module

キーボード入力に関する機能を提供する。

- down  
キーを押す
- up  
キーを放す
- press  
キー入力をする(押して放す)

### Example

例:Win+Shift-Sキーを押下する(結果、スクリーンキャプチャが起動する) 

```
key.down('win')
key.down('shift')
key.down('s')

key.up('s')
key.up('shift')
key.up('win')
```

例:'hello'を入力する
```
key.press(['h','e','l','l','o'])
```

### サポートしているキー

- ` `
  - スペース
- `!`
- `#`
- `$`
- `%`
- `&`
- `'`
- `(`
- `)`
- `*`
- `+`
- `,`
- `-`
- `.`
- `/`
- `0`
- `1`
- `2`
- `3`
- `4`
- `5`
- `6`
- `7`
- `8`
- `9`
- `:`
- `;`
- `<`
- `=`
- `>`
- `?`
- `@`
- `[`
- `\"`
- `\\`
- `\\n`
- `\\r\\n`
- `\\t`
- `]`
- `^`
- `_`
- `` ` ``
- `a`
- `alt`
- `apps`
- `b`
- `backspace`
- `c`
- `capslock`
- `convert`
- `ctrl`
- `d`
- `del`
- `delete`
- `down`
- `e`
- `end`
- `enter`
- `esc`
- `escape`
- `f10`
- `f11`
- `f12`
- `f13`
- `f14`
- `f15`
- `f16`
- `f17`
- `f18`
- `f19`
- `f20`
- `f21`
- `f22`
- `f23`
- `f24`
- `f2`
- `f3`
- `f4`
- `f5`
- `f6`
- `f7`
- `f8`
- `f9`
- `f`
- `g`
- `h`
- `home`
- `i`
- `insert`
- `j`
- `k`
- `kana`
- `kanji`
- `l`
- `left`
- `m`
- `n`
- `nonconvert`
- `num0`
- `num1`
- `num2`
- `num3`
- `num4`
- `num5`
- `num6`
- `num7`
- `num8`
- `num9`
- `numlock`
- `o`
- `p`
- `pagedown`
- `pageup`
- `pause`
- `printscreen`
- `q`
- `r`
- `return`
- `right`
- `s`
- `scrolllock`
- `shift`
- `t`
- `tab`
- `u`
- `up`
- `v`
- `w`
- `win`
- `x`
- `y`
- `z`
- `{`
- `|`
- `}`
- `~`

### win module

ウインドウに関する機能を提供する。

- find  
ウインドウを取得する


#### Example

例:ウインドウを探す

```
# メモ帳のウインドウを得る
w = win.find(class_name="Notepad")
# 最大化
w.maximize()
# 最小
w.minimize()
# 元に戻す
w.restore()
# 非表示
w.hide()
# 表示
w.show()
# 前面に表示
w.activate()
# 移動
w.move(x=0, y=0)
# リサイズ
w.resize(width=250, height=200)
# 移動とリサイズ
w.setpos(x=100,y=100,width=400,height=200)

```

## Per-Key Behavior

|押下キー|動作|
|--|--|
|`Enter`|スクリプトを実行する|

