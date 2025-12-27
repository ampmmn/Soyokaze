# Key Module

キーボード入力に関する機能を提供するモジュール。 
importしなくても使うことができる。

## Methods

### down  

キーを押す。  

```
# Enterキーを押す
key.down('enter')
# Enterキーを放す
key.up('enter')
```

#### Arguments

- chars(文字列)  
押すキーを表す文字。指定可能な文字列は[利用可能なキー文字列](#key-strings)を参照のこと

#### Return Value

常に0


###  up  

キーを放す。

```
# Enterキーを押す
key.down('enter')
# Enterキーを放す
key.up('enter')
```

#### Arguments

- chars(文字列)  
放すキーを表す文字。指定可能な文字列は[利用可能なキー文字列](#key-strings)を参照のこと

#### Return Value

常に0

### press  

キー入力をする(押して放す)。  
文字列を指定すると、単一のキー入力をする。   
リストを指定すると、リストの要素で指定したキー入力をする。  
また、繰り返し回数や間隔を指定することができる。

```
# Enterキーを入力する
key.press('enter')

# aiueoと入力する
key.press(['a', 'i' 'u', 'e', 'o'])

# aiueoを3回、0.5秒間隔で入力する
key.press(['a', 'i' 'u', 'e', 'o'], repeat=3, interval=0.5)

```

#### Arguments

- input(文字列またはリスト)  
放すキーを表す文字または文字列のリスト。指定可能な文字列は[利用可能なキー文字列](#key-strings)を参照のこと
- repeat(整数値)  
繰り返し回数。省略時は1
- interval(Float)
秒単位の間隔。省略時は0.0

#### Return Value

常に0

###  hotkey  

キーを同時押しする。  
ホットキー入力するのに使うことができる。

```
# Win-Shift-Sを同時押しする
key.hotkey('win', 'shift', 's')
```

#### Arguments

この関数は可変長引数で任意の数の文字列を指定できる。  
(入力するキーを表す文字列を指定する)

文字列でない引数、認識できない文字は無視される。

#### Return Value

常に0

###  write  

文字列を入力する。  
この関数で入力できるのは文字のみ。BackspaceやDelele、矢印キー、ファンクションキーや修飾キーなどの入力はできない。  

```
key.write('Hello World!', interval=0.1)
key.write("こんにちは\nこれは、テストです\n", interval=0.1)
```

#### Arguments

- input(文字列)  
入力する文字列
- interval(float)  
文字を入力する感覚を秒単位で指定する。省略時の値は0.0

#### Return Value

正常終了時は0、エラー時は1(第一引数が文字列でない場合)


## key strings

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

## Example

例:Win+Shift-Sキーを押下する(結果、スクリーンキャプチャが起動する) 

```python
key.down('win')
key.down('shift')
key.down('s')

key.up('s')
key.up('shift')
key.up('win')
```

`hotkey`メソッドを使うと、上記の操作が1回の呼び出しで済む

```python
key.hotkey('win', 'shift', 's')
```

例:'hello'を入力する
```
key.press(['h','e','l','l','o'])
```

writeメソッドを使うと文字列の形で入力できる

```python
key.write('hello')
```


