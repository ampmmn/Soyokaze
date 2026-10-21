# 271 単位付き計算機能

## 概要

Calculator候補に、単位付きの式を評価する機能を追加した。

この機能は、既存の`adhoc-command/unit-converter`にある単位変換コマンドとは別の機能である。
Calculator候補の評価処理から、単位付きの式を入力した場合にMathyPadを呼び出して結果を表示する。

標準電卓機能と単位付き計算機能は独立して有効・無効を設定できる。

## 関連ファイル

- `src/soyokaze/commands/calculator/Calculator.h`
- `src/soyokaze/commands/calculator/Calculator.cpp`
- `src/soyokaze/commands/calculator/CalculatorAdhocCommandProvider.cpp`
- `src/soyokaze/settingwindow/ExtensionSettingDialog.cpp`
- `src/soyokaze/Soyokaze.rc`
- `src/soyokaze/resource.h`
- `tests/testcode/soyokaze/commands/calculator/CalculatorTest.cpp`

## 設定値

設定画面の値は以下のキーへ保存する。

- `Calculator:Enable`
  - Pythonを利用する標準電卓機能の有効・無効
- `Calculator:IsUseUnitConverter`
  - 単位付き計算機能の有効・無効

`Calculator:IsUseUnitConverter`という名前は既存の設定名との互換性を維持するために使用している。
この設定は`Calculator:Enable`とは独立しているため、標準電卓を無効にして単位付き計算機能だけを有効にできる。

設定変更時は`CalculatorAdhocCommandProvider`が`Calculator`へ両方の設定を反映する。
アプリケーション起動後の候補準備時にも現在の設定を読み直す。

## 評価処理

`Calculator::Evaluate`は、以下の順序で式を評価する。

```text
標準電卓機能が有効
    -> Pythonによる標準電卓で評価
    -> 成功した場合は結果を返す

標準電卓機能が無効、または標準電卓で評価できない
    -> 単位付き計算機能で評価
    -> 成功した場合は結果を返す
```

標準電卓で評価に成功した場合は、単位付き計算機能を呼び出さない。
標準電卓が無効な場合や、入力が標準電卓で扱えない場合は、単位付き計算機能へフォールバックする。

単位付き計算機能も無効、または評価に失敗した場合は、`Calculator::Evaluate`が`false`を返す。

`CalculatorAdhocCommandProvider`は評価に失敗した式を候補として追加しない。
評価結果が整数文字列の場合だけ、従来どおり10進数・16進数・8進数・2進数の候補を追加する。
単位付き計算の結果が整数以外の場合は、10進数用の結果だけを追加する。

## 標準電卓機能の安全対策

標準電卓機能はPythonで式を評価するため、任意のコードを実行できてしまうと問題がある。
そのため、式の評価前に`Calculator::IsValidExpression`で検証を行い、
検証を通過した式のみをPythonへ渡す。

### 式の検証(ホワイトリスト方式)

`Calculator::PImpl::ValidateExpression`は、文字を先頭から読みながら以下の条件を確認する。

- 使用できる文字は、数字/小数点/英字/空白/`+`/`-`/`*`/`/`/`%`/`(`/`)`/`,` のみ
  - 引用符(`'`/`"`)は使えないため、文字列リテラルは記述できない
  - セミコロン(`;`)は使えないため、複文は記述できない
  - アンダースコア(`_`)は使えない
- 識別子(関数名・定数名・属性名)は許可されたものだけを使う
  - `math`モジュールの関数と定数
  - 電卓機能で使う組み込み関数(`abs`/`bin`/`float`/`hex`/`int`/`max`/`min`/`oct`/`round`)
  - 上記以外の識別子(組み込み関数や`__class__`などのdunder属性)は拒否する
  - 識別子が連続している場合(例: `1 2`)も拒否する
- 数値リテラルは最大30桁
  - 基数接頭辞(`0x`/`0o`/`0b`)にも対応する
  - 指数部は最大3桁、最大100
- べき乗(`**`)の指数は数値リテラルで最大100
  - 指数が入れ子になっている場合(例: `10**10**10`)は拒否する
  - これにより指数的な大きさの整数が生成されることを防ぐ
- 式の長さは最大256文字

`factorial`/`comb`/`perm`は、引数が大きいと評価結果が大きくなりリソースを過剰に消費するため、
使用できる識別子に含めていない。

### Python実行側の制限

`PythonProxy`側の電卓機能用グローバル辞書(`CreateCalcGlobalDict`)では、
`math`モジュールの関数・定数と上記の組み込み関数のみを登録する。
また、辞書の`__builtins__`を空の辞書に置き換えることで、
Pythonの組み込み関数を呼び出せないようにしている。

これにより、仮に式の検証を迂回できた場合でも、
Pythonの組み込み関数を起点に任意のコードへ到達できない。

### 評価結果の制限

`PythonProxy::EvalForCalculate`は、評価結果が整数/浮動小数点数/文字列以外の場合は
その結果を返さない(失敗として扱う)。

### タイムアウト

`PythonProxy::EvalForCalculate`は、3秒を超えて評価が終わらない場合に
`PyErr_SetInterrupt`で評価を中断する。

監視スレッドは`WaitForSingleObject`でカーネル上のイベントを待っている。
評価が完了した時点でイベントが通知され、監視スレッドが即座に解除される。
(イベントをポーリングで待つ実装では、監視スレッドの待ち間隔分の時間だけ
電卓の候補表示が遅れていた)

なお、Pythonの組込み関数を実行している場合はバイトコードの境界でしか中断できないため、
`PyErr_SetInterrupt`では停止できない。
そのため、計算量が大きくなり得る式は式の検証段階で拒否している。

### 文字列表現の取得

`PythonProxy::py2str`は、オブジェクトの文字列表現を取得できない場合に空文字列を返す。
桁数の多い整数(例: `sys.set_int_max_str_digits`の既定値を超える桁数)では
`PyObject_Repr`が失敗するため、その場合は候補として採用しない。

## MathyPad DLL連携

`Calculator::PImpl::InitializeMathypad`で、アプリケーションのモジュールファイルディレクトリにある`mathypad.dll`を遅延ロードする。
DLLは最初に単位付き計算を実行するときにロードし、一度初期化を試みた後は同じ`Calculator`インスタンスで再試行しない。

DLLから以下のAPIを取得する。

- `mathypad_evaluate`
- `mathypad_free_string`

APIのどちらか一方でも取得できない場合は初期化に失敗したものとしてDLLを解放し、単位付き計算を利用不可とする。

`mathypad_evaluate`の関数定義は以下のとおりである。

```cpp
char* mathypad_evaluate(const char* expression);
```

返却された文字列は`mathypad_free_string`で解放する。
解放前に`std::string`へコピーし、その後にSoyokaze側の文字列へ変換する。

`Calculator`の破棄時には、`FreeLibrary`でDLLを解放する。

## 文字コード

MathyPad APIの引数と返却値はUTF-8として扱う。

1. `CString`の入力をUTF-8へ変換する
2. UTF-8文字列を`mathypad_evaluate`へ渡す
3. 返却されたUTF-8文字列をコピーする
4. `mathypad_free_string`で返却バッファを解放する
5. Soyokaze側の文字列へ変換する

MathyPadが`nullptr`を返した場合は評価失敗とする。

## 対応単位

MathyPadによる単位付き計算では、以下の単位を扱う。

### 時間

- ナノ秒
- ミリ秒
- 秒
- 分
- 時
- 日
- 週
- 月
- 四半期
- 年

### ビット単位

- `Bit`、`Kb`、`Mb`、`Gb`、`Tb`、`Pb`、`Eb`
- `Kib`、`Mib`、`Gib`、`Tib`、`Pib`、`Eib`

### バイト単位

- `Byte`、`KB`、`MB`、`GB`、`TB`、`PB`、`EB`
- `KiB`、`MiB`、`GiB`、`TiB`、`PiB`、`EiB`

### パーセント

- `%`

## テスト

`CalculatorTest`では、標準電卓機能と単位付き計算機能の両方を無効にした場合に、評価が失敗することを確認する。

`Calculator::IsValidExpression`を使って、式の検証を単体テストしている。

- 四則演算や`math`モジュールの関数・定数、べき乗は評価可能な式として扱う
- `import`や`eval`等の組み込み関数、dunder属性を経由した操作、引用符やセミコロンを含む式は拒否する
- 評価結果が指数的に大きくなる式(例: `10**10**10`)は拒否する
- 数値リテラルや式の長さが上限を超える式は拒否する

MathyPad DLLの実体に依存する評価は、テスト環境へDLLを配置する必要があるため、通常の単体テストでは直接検証していない。
DLLを使用する評価を追加でテストする場合は、DLLの配置、APIのエクスポート、返却文字列の解放まで確認する必要がある。

## 実装上の注意点

- 標準電卓機能と単位付き計算機能の設定を同じ条件として扱わない
- `mathypad.dll`のロード失敗をアプリケーション全体の初期化失敗にしない
- MathyPadが返した文字列をSoyokaze側で解放しない
- `mathypad_free_string`による解放前に返却内容をコピーする
- DLL APIの取得に失敗した場合は、取得済みのハンドルと関数ポインタをすべて破棄する
- 単位付き計算結果に対して、整数用の基数変換候補を無条件に追加しない
- 電卓機能ではPythonの組み込み関数を呼び出せないようにする
  - 電卓機能用のグローバル辞書の`__builtins__`は空の辞書にする
  - 使用する組み込み関数は個別に登録する
- 電卓機能の式はホワイトリストで検証してからPythonへ渡す
  - 組み込み関数の名称を拒否するリストではなく、許可する識別子を列挙する
- 電卓機能とPython拡張コマンドは同じインタープリタで実行する
  - サブインタープリタを使用すると`PyErr_SetInterrupt`で評価を中断できないため
- タイムアウトの監視はカーネル上のイベント待ちで実装する
  - ポーリングで待つと、監視スレッドの待ち間隔の分だけ評価の完了が遅れる
  - Python拡張コマンド側は、中止要求(`ProxyWindow::Abort()`)に10ms以内で反応するため、
    10ms単位で分割した待ちを行う
