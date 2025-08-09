# Calculator

入力欄に数式を入れると、計算結果をコメント欄に表示する。  
Enterキーを押下すると、計算結果をクリップボードにコピーできる。

利用するにはPythonが必要。([設定画面](/window/app-settings.md#extensions))  

![](../image/adhoc-command/calculator1.png)

## 利用可能な演算子

|演算子|意味|
|----|----|
|+|加算|
|-|減算|
|*|乗算|
|/|除算|
|//|切り捨て除算|
|**|べき乗|
|<<,>>|シフト演算|
|%|剰余|
|&|AND|
|\||OR|
|^|XOR|

## Function

その他、Python標準関数のうち、計算結果が文字列/数値になるものと、[math](https://docs.python.org/ja/3/library/math.html)モジュールの関数を使用することができる。

![](../image/calculator2.png)

## Radix-Based Number Display

計算結果が整数になる場合は、2進数/8進数/10進数/16進数それぞれの結果が表示される。

![](../image/adhoc-command/calculator2.png)



