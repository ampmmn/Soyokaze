# SearchViewer

SearchViewer は、Web 検索結果を表示する Windows 向けのビューアです。WebView2 を使用し、検索プロファイルごとに専用のウインドウとユーザーデータを使用します。

一般的なWebブラウザーの代替ではなく、検索結果の確認を目的としています。

## 起動方法

```text
SearchViewer.exe --profile=google "明日の天気"
SearchViewer.exe --profile=github --config="C:\Tools\SearchViewer.yaml" "webview2"
SearchViewer.exe --profile=google --profile=bing "明日の天気"
```

### コマンドライン引数

- `--profile <名前>` または `--profile=<名前>`

  検索に使用するプロファイルを指定します。複数指定すると、それぞれのプロファイルで検索結果を表示します。同じプロファイルを重複して指定することはできません。

- `--config <パス>` または `--config=<パス>`

  設定ファイルを指定します。省略時は `SearchViewer.exe` と同じフォルダーにある `SearchViewer.yaml` を使用します。

- `検索語`

  検索語は1つの引数として指定してください。空白を含む場合は、上記の例のように引用符で囲みます。検索語はURLに挿入する際に自動的にURLエンコードされます。

プロファイル名には、半角英数字、`_`、`-` を使用できます。長さは1～16文字です。

同じプロファイルですでにウインドウが開いている場合、新しいウインドウは作成されず、そのウインドウで検索結果を更新します。ウインドウが最小化されていれば復元されます。起動中のウインドウと異なる設定ファイルを指定した場合は、検索要求がエラーになります。

## 設定ファイルの作成・編集

設定ファイルは UTF-8 形式の YAML ファイルです。既定のファイル名は `SearchViewer.yaml` で、`SearchViewer.exe` と同じフォルダーに置きます。別の場所に置く場合は、起動時に `--config` でパスを指定してください。指定したファイルが存在しない場合はエラーになります。

### 設定例

```yaml
profiles:
  google:
    site: google
    title: Google検索
  bing:
    site: bing
    title: Bing検索
  github:
    site: github
    title: GitHubコード検索

sites:
  google:
    searchUrl: "https://www.google.com/search?q={query}"
  bing:
    searchUrl: "https://www.bing.com/search?q={query}"
  github:
    searchUrl: "https://github.com/search?q={query}"
```

### 設定項目

- `profiles`

  利用するプロファイルを定義します。キーが `--profile` に指定する名前です。各プロファイルには、参照するサイト名を `site` に、ウインドウのタイトルを `title` に指定します。

- `sites`

  検索サイトを定義します。プロファイルの `site` と同じ名前の項目を作成し、検索URLを `searchUrl` に指定します。

- `searchUrl`

  検索語を挿入する位置に `{query}` を含めてください。URLには `http://` または `https://` を指定します。検索語は自動的にURLエンコードされます。

1つのサイト定義を複数のプロファイルから参照できます。たとえば、同じ検索サイトを使いながら、プロファイルごとに異なるウインドウタイトルを設定できます。

## ウインドウと閲覧データ

- ウインドウの位置と表示状態は、実行ファイルと同じフォルダーの `SearchViewerWindow.yaml` に自動保存されます。
- Cookie、セッション情報などのWebView2ユーザーデータは、`UserData/<プロファイル名>` に保存されます。データを初期化する場合は、SearchViewer を終了してから対象プロファイルのフォルダーを削除してください。

設定ファイル、ウインドウ位置ファイル、ユーザーデータは、実行ファイルと同じフォルダー以下に保存するため、その場所にファイルやフォルダーを作成できる必要があります。

## 操作と制限

- 検索結果内のリンクは同じウインドウ内で開きます。
- アドレス欄には現在のページのURLが表示されます。読み取り専用のため、URLを入力して任意のページを開くことはできません。
- 表示できるページのURLは HTTP または HTTPS に限られます。
- 戻る、進む、再読み込み、ページ内検索には、それぞれ `Alt`+`←`、`Alt`+`→`、`F5`、`Ctrl`+`F` を使用できます。
- Windows 専用です。一般的なブラウザーのタブ、お気に入り、ダウンロード管理などは備えていません。
