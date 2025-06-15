# ビルド方法

## 開発環境

- VisualStudio2022
- C++のプロジェクト
- MFCを使っている

ユニットテストフレームワークとしてgoogletest(1.13.0)を用いている。

## ソースコードを取得する

```
git clone --recursive https://github.com/ampmmn/Soyokaze.git
```

## ビルド時に必要な外部ライブラリ

以下のライブラリに依存している。  
`git submoudle`で管理しているので、前述の`git clone --recursive`により、
本アプリのソースコードと合わせて、一括で取得できる。

- [nlohmann-json](https://github.com/nlohmann/json)
  - JSONを読むために利用している。
  - ヘッダファイルベースのライブラリなので、このライブラリをビルドする必要はなく、配置するだけでよい。

- [fmt](https://github.com/fmtlib/fmt)
  - 便利な汎用の文字列フォーマットライブラリ
  - VisualStudio2022でビルドする分にはC++20の標準ライブラリを使えばよいのだけれど、VisualStudio2017でもビルドしたいのでこちらを利用する
  - ヘッダファイルベースのライブラリなので、このライブラリをビルドする必要はなく、配置するだけでよい。

- [spdlog](https://github.com/gabime/spdlog)
  - ログ出力ライブラリ
  - このライブラリは内部でfmtを利用しているが、ライブラリ自身が内包している方を使う構成としている
  - 要ビルド

- [tidy-html5](https://github.com/htacg/tidy-html5)
  - HTML文書を修正するためのライブラリであるが、HTMLをパースしてノードの情報を取得することもできるので、その用途で利用している
  - ライブラリ部(libtidy)を利用している
  - 要ビルド

- [re2](https://github.com/google/re2)
  - 正規表現ライブラリ。コマンド名のマッチングの用途で使用している
  - `0.40.0`まではC++標準ライブラリの正規表現ライブラリを使っていたが、こちらに移行した

- [Abseil](https://github.com/abseil/abseil-cpp)
  - Google製のC++ライブラリ群。re2がこれに依存している。

### 外部ライブラリの配置

- fmt
  - https://github.com/fmtlib/fmt からソース一式を取得する
    - `externals`ディレクトリに`fmt`を配置する
- nlohmann-json
  - https://github.com/nlohmann/json からソース一式を取得する
    - `externals`ディレクトリに`nlohmann-json`を配置する
- spdlog
  - https://github.com/gabime/spdlog からソース一式を取得する
    - `externals`ディレクトリに`spdlog`を配置する
- tidy-html5
  - https://github.com/htacg/tidy-html5 からソース一式を取得する
    - `externals`ディレクトリに`tidy-html5`を配置する
- re2
  - https://github.com/google/re2 からソース一式を取得する
    - `externals`ディレクトリに`re2`を配置する
- Abseil
  - https://github.com/abseil/abseil-cpp からソース一式を取得する
    - `externals`ディレクトリに`abseil-cpp`を配置する

- `Soyokaze`のプロジェクト設定にて、`externals`ディレクトリに`json` `spdlog`というフォルダがあることを想定している  
以下のように置く  
↓
```
soyokaze-src/
  src/
     Soyokaze.sln
  externals/
    fmt/
      include/
    json/
      include/
        nlohmann/
          json.hpp
    spdlog/
      include/
    tidy-html5/
      include/
    re2/
    abseil-cpp/
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

### abseilのビルド

配置した`abseil-cpp`を下記コマンドでビルドする。  
cmakeは`vcvarsall.bat amd64`を実行すればパスが通る想定  

```
cd externals/abseil-cpp
mkdir -p build
cd build

# ランチャー側の「Release」構成で利用するモジュールのビルド
cmake -DCMAKE_INSTALL_PREFIX=install ..
cmake --build . --config Release
cmake --install .
cd install\lib
# ランチャー側で「absl_all.lib」という名前でリンクを試みるため、すべての.libを連結しておく
lib.exe /out:absl_all.lib *.lib

cd ..\..

# ランチャー側の「ReleaseStatic」構成で利用するモジュールのビルド
cmake -DCMAKE_INSTALL_PREFIX=install -DABSL_MSVC_STATIC_RUNTIME=ON -DCMAKE_RELEASE_POSTFIX=rtst ..
cmake --build . --config Release --clean-first
cmake --install .
cd install\lib
# ランチャー側で「absl_allrtst.lib」という名前でリンクを試みるため、すべての.libを連結しておく
lib.exe /out:absl_allrtst.lib *rtst.lib

# ランチャー側の「Debug」「UnitTest」構成で利用するモジュールのビルド
cmake -DCMAKE_INSTALL_PREFIX=install -DABSL_MSVC_STATIC_RUNTIME=OFF -DCMAKE_RELEASE_POSTFIX=_d ..
cmake --build . --config Debug --clean-first
cmake --install . --config Debug
cd install\lib
# ランチャー側で「absl_all_d.lib」という名前でリンクを試みるため、すべての.libを連結しておく
lib.exe /out:absl_all_d.lib *_d.lib

```

### re2のビルド

配置した`re2`を下記コマンドでビルドする。  
cmakeは`vcvarsall.bat amd64`を実行すればパスが通る想定  

```
cd externals/re2
mkdir -p build
cd build

# ランチャー側の「Release」構成で利用するモジュールのビルド
cmake -DCMAKE_INSTALL_PREFIX=install -DCMAKE_PREFIX_PATH=..\abseil-cpp\build\install ..
cmake --build . --config Release
cmake --install .

# ランチャー側の「ReleaseStatic」構成で利用するモジュールのビルド
cmake -DCMAKE_INSTALL_PREFIX=install -DCMAKE_PREFIX_PATH=..\abseil-cpp\build\install -DCMAKE_RELEASE_POSTFIX=rtst -DCMAKE_CXX_FLAGS_RELEASE:STRING="/MT /O2 /Ob2 /DNDEBUG" ..
cmake --build . --config Release --clean-first
cmake --install .

# ランチャー側の「Debug」「UnitTest」構成で利用するモジュールのビルド
cmake -DCMAKE_INSTALL_PREFIX=install -DCMAKE_PREFIX_PATH=..\abseil-cpp\build\install -DCMAKE_DEBUG_POSTFIX=_d -DCMAKE_CXX_FLAGS_DEBUG:STRING="/Od /Ob0 /EHsc /RTC1 /MDd" ..
cmake --build . --config Debug --clean-first
cmake --install . --config Debug
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

