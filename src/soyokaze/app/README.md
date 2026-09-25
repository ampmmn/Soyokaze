# app

アプリケーションの起動、コマンドライン、二重起動連携、OSイベント、マニュアル表示に関する処理を置くディレクトリ。

## クラス

|クラス名|説明|
|---|---|
|[LauncherApp](./LauncherApp.h)|MFCアプリケーションの起点。単一起動判定、保存先モード確定、ログ初期化、通常起動と後続起動の振り分けを行う。|
|[FirstStartDialog](./FirstStartDialog.h)|初回起動時に通常モードかポータブルモードかを選択するダイアログ。|
|[AppProcess](./AppProcess.h)|名前付きミューテックスでプロセスの多重起動を検知し、必要に応じて管理者権限で再起動する。|
|[Arguments](./Arguments.h)|`argv`を保持し、オプションや値の検索・削除を行う低レベルの引数処理。|
|[StartupParam](./StartupParam.h)|起動オプションをアプリ固有の操作（コマンド実行、パス登録、表示状態、テキスト入力、選択範囲、作業ディレクトリ変更など）として解釈する。|
|[CommandLineProcessor](./CommandLineProcessor.h)|後続起動時の引数を解釈し、先行プロセスへの要求に変換する。|
|[SecondProcessProxyIF](./SecondProcessProxyIF.h)|後続プロセスから先行プロセスへ要求を送るインターフェース。|
|[SecondProcessProxy](./SecondProcessProxy.h)|プロセス間メッセージキューを介して、コマンド送信、パス登録、作業ディレクトリ変更、表示・非表示などを先行プロセスに依頼する。|
|[LauncherEventDispatcher](./LauncherEventDispatcher.h)|イベントリスナーを管理し、タイマー、セッション、モニター構成、ランチャーのアクティブ状態などの通知を配信する。|
|[LauncherSystemEventWindow](./LauncherSystemEventWindow.h)|セッションロック／解除、定期タイマー、ディスプレイ構成変更を受け取り、イベントディスパッチャーへ通知する不可視ウインドウ。|
|[LauncherShutdownWindow](./LauncherShutdownWindow.h)|Windowsの終了セッション通知を受け取り、終了処理を開始する不可視ウインドウ。|
|[Manual](./Manual.h)|マニュアルのページIDをローカルHTMLの場所に対応付け、ページ表示を依頼するシングルトン。|
|[ManualWindow](./ManualWindow.h)|内部ブラウザーを専用スレッド上に作成し、マニュアルを表示するシングルトン。|
|[SQLiteMemoryUsedMetrics](./metrics/SQLiteMemoryUsedMetrics.h)|SQLiteのメモリ使用量をリソースメトリクスとして提供する。|

`AppProcess::exception`は、単一起動用ミューテックスの初期化失敗を伝える例外型。PImplを持つクラスは、実装詳細を各`.cpp`側に隠蔽する。

## 初回起動時の保存先選択

初回起動時は、プロファイル保存先を確定してから、設定ファイルやログを読む。初期化順序を変える場合は、保存先を参照するコードが選択処理より先に実行されないことを確認する。

```text
LauncherApp::InitInstance()
  ├─ AppProcess::Exists() で先行プロセスの有無を判定
  ├─ 先行プロセスがなければ MFC と共通コントロールを初期化
  ├─ CAppProfile::InitializeProfileMode()
  │    ├─ 既存モードを検出 → 既存の保存先を使用
  │    └─ 未検出 → FirstStartDialog を表示
  │         ├─ キャンセル → 終了
  │         └─ 選択 → SetRunAsPortable() → EnsureProfileRoot()
  ├─ Logger などを初期化
  └─ InitFirstInstance() → CreateUserDirectory() で追加の設定フォルダを準備
```

既存保存先の判定では、実行ファイル隣の`profile`をユーザープロファイル保存先より優先する。通常モードは`USERPROFILE`直下の`APP_PROFILE_DIRNAME`、ポータブルモードは実行ファイル隣の`profile`を使用する。選択ダイアログでキャンセルした場合、または選択先を作成できない場合は、通常起動へ進まず終了する。

二重起動側は、保存先モードを確認してからコマンドライン要求を先行プロセスへ転送する。初回選択が進行中でまだ保存先が確定していない場合は、案内を表示して終了する。

### ダイアログとリソースの対応

`FirstStartDialog::DoDataExchange()`は、`IDC_RADIO_SAVETOUSERPROFILE`を起点に`DDX_Radio`で選択値を読み書きする。値`0`は通常モード、値`1`はポータブルモードを表す。`OnOK()`で`UpdateData()`を呼び出してからダイアログを閉じるため、リソース上のラジオボタンのグループとタブオーダーを維持すること。タブオーダーが崩れると、画面上の選択とDDXの値が一致しないことがある。

ダイアログ定義はリソーススクリプト（`.rc`）、コントロールIDは`resource.h`にある。画面文言、DDXの起点、ラジオボタンの並びを変更した場合は、通常モードとポータブルモードの両方を実際に選択して確認する。

### 保存先を早期確定しないこと

`CAppProfile::GetDirPath()`や`Path(Path::APPDIR, ...)`は、内部で`GetProfileDirRoot()`を呼び出す。モード未確定時にこの経路へ入ると、実行ファイル隣の`profile`がなければ通常モードとして`PROFILE_NORMAL`が設定され、その後の初回選択が既存モード判定として扱われてしまう。

このため、プロファイルを読む初期化処理は保存先モード確定後まで遅延する。たとえば`PluginProvider::PImpl`のコンストラクタではリスナー登録だけを行い、`PluginSettings::Load()`は`LoadPlugins()`の遅延処理内で呼ぶ。設定変更時の`OnAppPreferenceUpdated()`による再読み込みは、確定済みの保存先を使う。

`AppPreference::CreateUserDirectory()`は選択後の`InitFirstInstance()`から呼び出し、プロファイルルート、PCごとの設定ディレクトリ、起動通知を準備する。これより前にプロファイル依存のリスナー処理を追加しないこと。

## その他の構成要素

|ファイル／機能|説明|
|---|---|
|[LocalDirectoryWatcherService](./LocalDirectoryWatcherService.h)|`Start()`でアプリ設定の終了通知リスナーを登録し、終了時にローカルディレクトリ監視を終了する。|
|[AppName.h](./AppName.h)|アプリ名、実行ファイル名、プロファイルディレクトリ名などの定数・マクロを定義する。|
|[override.h](./override.h)|起動登録の無効化など、必要に応じて使うビルド時の上書き定義を置く。|

`LocalDirectoryWatcherService.cpp`内の`FinalizeCaller`は、設定終了通知を受けて監視サービスを終了する内部リスナー。
