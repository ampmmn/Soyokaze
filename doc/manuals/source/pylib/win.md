# Win Module

ウインドウに関する機能を提供するモジュール。  
importしなくても使うことができる。

## Methods

### find  

ウインドウクラスやタイトルを指定して、条件に合致するウインドウを取得する。

```python

# ウインドウクラス名が"Notepad"のウインドウを得る
w1 = win.find(class_name="Notepad")

# タイトルが"Hoge"のウインドウを得る
w2 = win.find(caption="Hoge")
```

#### Arguments

- `class_name`(文字列)  
ウインドウクラス名。省略時はNone(文字列)
- `caption`(文字列)  
ウインドウタイトル。省略時はNone(文字列)
- `timeout`(数値)  
ウインドウが見つからなかった場合に見つかるまで待機する時間を秒単位で指定する。省略時は0.0

#### Return Value

ウインドウオブジェクトが返る。  
ウインドウが存在しない場合でもオブジェクトが返る。ウインドウの有無は戻り値で得たオブジェクトの`exists`プロパティで判断する。

ウインドウオブジェクトの提供するメソッドは下記の例を参照のこと

## Window class

`win.find()`メソッドの戻り値で得られるオブジェクト。  
対応するウインドウを操作する機能を提供する。

## Properties

### exists

ウインドウの有無を表すBool型の値

```
# メモ帳のウインドウを得る
w = win.find(class_name="Notepad")
if w.exists:
    # ウインドウは存在する
    pass
else:
    # ウインドウは存在しない
    pass
```

## Methods

### maximize

ウインドウを最大化する。

```python
# メモ帳のウインドウを得る
w = win.find(class_name="Notepad")
# 最大化
w.maximize()
```

#### Arguments

なし

#### Return Value

- True: 成功
- False: 失敗

### minimize

ウインドウを最小化する。

```python
# メモ帳のウインドウを得る
w = win.find(class_name="Notepad")
# 最小化
w.minimize()
```

#### Arguments

なし

#### Return Value

- True: 成功
- False: 失敗

### restore

最大化/最小化状態にあるウインドウを通常表示の状態に戻す。

```python
# メモ帳のウインドウを得る
w = win.find(class_name="Notepad")
# 通常表示にする
w.restore()
```

#### Arguments

なし

#### Return Value

- True: 成功
- False: 失敗

### hide

ウインドウを非表示にする。

```python
# メモ帳のウインドウを得る
w = win.find(class_name="Notepad")
# ウインドウを隠す
w.hide()
```

#### Arguments

なし

#### Return Value

- True: 成功
- False: 失敗

### show

ウインドウを表示する。

```python
# メモ帳のウインドウを得る
w = win.find(class_name="Notepad")
# ウインドウを表示
w.show()
```

#### Arguments

なし

#### Return Value

- True: 成功
- False: 失敗


### activate

ウインドウを前面に出しフォーカスを設定する。

```python
# メモ帳のウインドウを得る
w = win.find(class_name="Notepad")
# ウインドウを前面に出す
w.activate()
```

#### Arguments

なし

#### Return Value

- True: 成功
- False: 失敗


### move

ウインドウを指定した座標値に移動する

```python
# メモ帳のウインドウを得る
w = win.find(class_name="Notepad")
# 移動
w.move(x=0, y=0)
```

#### Arguments

- x(整数値)  
X座標値
- y(整数値)  
Y座標値

#### Return Value

- True: 成功
- False: 失敗


### resize

ウインドウを指定したサイズに変更する

```python
# メモ帳のウインドウを得る
w = win.find(class_name="Notepad")
# サイズ変更
w.resize(width=250, height=200)
```

#### Arguments

- width(整数値)  
ウインドウの幅
- height(整数値)  
ウインドウの高さ

#### Return Value

- True: 成功
- False: 失敗

### setpos

ウインドウの移動とリサイズを同時に行う。

```python
# メモ帳のウインドウを得る
w = win.find(class_name="Notepad")
# 移動とリサイズ
w.setpos(x=100,y=100,width=400,height=200)
```

#### Arguments

- insert_after  
Zオーダー変更用の引数。現在は非サポート。
- x(整数値)  
X座標値
- y(整数値)  
Y座標値
- width(整数値)  
ウインドウの幅
- height(整数値)  
ウインドウの高さ

#### Return Value

- True: 成功
- False: 失敗

### click

ウインドウ上をクリックする

```python
# メモ帳のウインドウを得る
w = win.find(class_name="Notepad")
# クリック
w.click(x=5, y=5)
```

#### Arguments

- x(整数値)  
クリック位置を表すX座標値。ウインドウのクライアント領域座標単位で指定する。
- y(整数値)  
クリック位置を表すY座標値。ウインドウのクライアント領域座標単位で指定する。

#### Return Value

- True: 成功
- False: 失敗

