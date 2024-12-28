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

- [spdlog](https://github.com/gabime/spdlog)
  - ログ出力ライブラリ  
要ビルド

- [tidy-html5](https://github.com/htacg/tidy-html5)
  - HTML文書を修正するためのライブラリであるが、HTMLをパースしてノードの情報を取得することもできるので、その用途で利用している
  - ライブラリ部(libtidy)を利用している  
要ビルド

### 外部ライブラリの配置

- nlohmann-json
  - https://github.com/nlohmann/json からソース一式を取得する
    - `externals`ディレクトリに`nlohmann-json`を配置する
- spdlog
  - https://github.com/gabime/spdlog からソース一式を取得する
    - `externals`ディレクトリに`spdlog`を配置する
- tidy-html5
  - https://github.com/htacg/tidy-html5 からソース一式を取得する
    - `externals`ディレクトリに`tidy-html5`を配置する

- `Soyokaze`のプロジェクト設定にて、`externals`ディレクトリに`json` `spdlog`というフォルダがあることを想定している  
以下のように置く  
↓
```
soyokaze-src/
  src/
     Soyokaze.sln
  externals/
    json/
      include/
        nlohmann/
          json.hpp
    spdlog/
      include/
    tidy-html5/
      include/
```

### spdlogのビルド

配置した`spdlog`を下記コマンドでビルドする。  
cmakeは`vcvarsall.bat amd64`を実行すればパスが通る想定  

```
cd externals/spdlog
mkdir build
cd build
cmake ..
msbuild /m /p:Configuration=Release /p:Platform=x64 spdlog.sln 
```

### tidy-html5(libtidy)のビルド

配置した`tidy-html5`を下記コマンドでビルドする。  
cmakeは`vcvarsall.bat amd64`を実行すればパスが通る想定  

```
cd externals/tidy-html5/build/cmake

# ランチャー側の「Debug」「Release」「UnitTest」構成で利用するモジュールのビルド
cmake ../.. -DCMAKE_BUILD_TYPE=Release -A x64
cmake --build . --config Release

# ランチャー側の「ReleaseStatic」構成で利用するモジュールのビルド
del CMakeCache.txt
cmake ../.. -DCMAKE_BUILD_TYPE=Release -DUSE_STATIC_RUNTIME=ON -DCMAKE_RELEASE_POSTFIX=rtst -A x64
cmake --build . --config Release

```

## ビルド方法

- VisualStudioでソリューションファイル`src/Soyokaze.sln`を開く
  - メニューの `ファイル`→`開く`→`プロジェクト/ソリューション`

- プロジェクト設定の下記項目を変更する必要があるのでビルド環境にあわせて設定する
  - `構成プロパティ`→`全般`→`Windows SDKバージョン`
  - `構成プロパティ`→`全般`→`プラットフォームツールセット`

- ビルドを実行する
  - メニューの `ビルド`→`ソリューションのビルド`

- ビルドが完了すると下記の場所にモジュールがてきる
  - `dist`/`x64`/(`Debug` or `Release` or `ReleaseStatic`)
    - 64bit版モジュールの場合
    - 32bit版モジュールのビルドは非サポート

## ソリューション構成

- Debug → デバッグ情報あり
- Release → デバッグ情報なし、最適化してる、共有DLLでMFCを使う
- ReleaseStatic → デバッグ情報なし、最適化してる、スタティックライブラリでMFCを使う。
- UnitTest → ユニットテストのビルド用
  - 本体のexe(soyokaze.exe)を.libとしてビルドし、ユニットテスト側の.exeにリンクする構成

