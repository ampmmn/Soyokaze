# ビルド方法

## 開発環境

- VisualStudio2022
- C++のプロジェクト
- MFCを使っている

ユニットテストフレームワークとしてgoogletest(1.13.0)を用いている。

## ビルド時に必要な外部ライブラリ

- [nlohmann-json](https://github.com/nlohmann/json)
  - JSONを読むために利用している。  
ヘッダファイルベースのライブラリなので、このライブラリをビルドする必要はなく、配置するだけでよい。

### nlohmann-jsonの配置

- https://github.com/nlohmann/json からソース一式を取得する
- `Soyokaze`のソースファイル一式と同じ階層に`nlohmann-json`を配置する

- `Soyokaze`のプロジェクト設定にて、`Soyokaze.sln`と同じ階層に`json`というフォルダがあることを想定している  
以下のように置く  
↓
```
soyokaze-src/
  Soyokaze.sln
  json/
    include/
      nlohmann/
        json.hpp
```

## ビルド方法

- VisualStudioでソリューションファイル`Soyokaze.sln`を開く
  - メニューの `ファイル`→`開く`→`プロジェクト/ソリューション`

- プロジェクト設定の下記項目を変更する必要があるのでビルド環境にあわせて設定する
  - `構成プロパティ`→`全般`→`Windows SDKバージョン`
  - `構成プロパティ`→`全般`→`プラットフォームツールセット`

- ビルドを実行する
  - メニューの `ビルド`→`ソリューションのビルド`

- ビルドが完了すると下記の場所にモジュールがてきる
  - `x64`/(`Debug` or `Release` or `ReleaseStatic`)
    - 64bit版モジュールの場合

## ソリューション構成

- Debug → デバッグ情報あり
- Release → デバッグ情報なし、最適化してる、共有DLLでMFCを使う
- ReleaseStatic → デバッグ情報なし、最適化してる、スタティックライブラリでMFCを使う。
- Release-Portable→ デバッグ情報なし、最適化してる、スタティックライブラリでMFCを使う。設定情報を`soyokaze.exe`と同じフォルダ以下に保存する
- ReleaseStatic-Portable → デバッグ情報なし、最適化してる、スタティックライブラリでMFCを使う。設定情報を`soyokaze.exe`と同じフォルダ以下に保存する
- UnitTest → ユニットテストのビルド用
  - 本体のexe(soyokaze.exe)を.libとしてビルドし、ユニットテスト側の.exeにリンクする構成

