# Third Party Libraries

```{only} soyokaze
- [fmt](https://github.com/fmtlib/fmt)
  - 文字列フォーマット用

- [nlohmann-json](https://github.com/nlohmann/json)
  - jsonファイルをパースするために使用している

- [spdlog](https://github.com/gabime/spdlog)
  - 実行時ログを出力するためのライブラリとして使用している

- [tidy-html5](https://github.com/htacg/tidy-html5)
  - DirectoryIndexコマンド内において、取得したHTMLを解析するためのライブラリとして使用している

- [C/Migemo](https://github.com/koron/cmigemo)
  - 日本語をローマ字検索するためのライブラリとして使用している

- [yet-another-migemo-dict](https://github.com/oguna/yet-another-migemo-dict)
  - 上記ライブラリから生成したMigemo辞書を本ツールに同梱している
  - アプリ実行時に上記ライブラリを直接利用するものではない

- [Python3](https://www.python.org/)
  - 簡易電卓機能を実現するために使用している

- [SQLite3](https://www.sqlite.org/)
  - Windows標準搭載の`winsqlite3.dll`を利用している

- [Everything SDK](https://www.voidtools.com/support/everything/sdk/)
  - `Everything`が公開しているSDKのソースコードをそのままランチャー本体に組み込んでいる
  - 現在はEverything関連機能を無効化している。そのうち再実装するつもり

- [RE2](https://github.com/google/re2)
  - コマンドの絞り込みの際の正規表現エンジンとして利用している

- [Abseil](https://github.com/abseil/abseil-cpp)
  - `RE2`がライブラリに依存しているライブラリ。本アプリは直接利用していない。
- [Sphinx](https://www.sphinx-doc.org/)
  - マニュアル生成で利用している
- [sphinx_rtd_theme](https://github.com/readthedocs/sphinx_rtd_theme)
  - マニュアル生成の際のテーマ

```

```{only} not soyokaze
非開示
```

