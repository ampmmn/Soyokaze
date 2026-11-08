#include "pch.h"
#include "OutputPreviewCommand.h"
#include "OutputPreviewEditor.h"
#include "OutputPreviewRuntime.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/output_preview/OutputPreviewResultCommand.h"
#include "actions/builtin/CallbackAction.h"
#include "actions/builtin/OpenPathInFilerAction.h"
#include "actions/builtin/ShowPropertiesAction.h"
#include "commands/output_preview/OutputPreviewTextDecoder.h"
#include "core/IFIDDefine.h"
#include "matcher/Pattern.h"
#include "features/main/controller/MainWindowController.h"
#include "icon/IconLoader.h"
#include "utility/CharConverter.h"
#include "utility/CommandLineArgument.h"
#include "utility/FileAssociationResolver.h"
#include "utility/LocalPathResolver.h"
#include "utility/Path.h"
#include "utility/LastErrorString.h"
#include "core/LauncherProcessContext.h"
#include "resource.h"
#include <fstream>
#include <filesystem>
#include <limits>

namespace launcherapp {
namespace commands {
namespace output_preview {

namespace {

class ScopedHandle
{
public:
	/**
	  管理対象のハンドルを保持する
	  @param[in] handle 管理対象
	*/
	explicit ScopedHandle(HANDLE handle = nullptr) : mHandle(handle)
	{
	}
	/**
	  管理対象のハンドルを閉じる
	*/
	~ScopedHandle()
	{
		if (mHandle && mHandle != INVALID_HANDLE_VALUE) {
			CloseHandle(mHandle);
		}
	}
	ScopedHandle(const ScopedHandle&) = delete;
	ScopedHandle& operator=(const ScopedHandle&) = delete;
	/**
	  保持中のハンドルを取得する
	  @return ハンドル
	*/
	HANDLE Get() const { return mHandle; }
	/**
	  保持中のハンドルを置き換える
	  @param[in] handle 新しいハンドル
	*/
	void Reset(HANDLE handle = nullptr)
	{
		if (mHandle && mHandle != INVALID_HANDLE_VALUE) {
			CloseHandle(mHandle);
		}
		mHandle = handle;
	}

private:
	HANDLE mHandle;
};

class RuntimeCloseGuard
{
public:
	/**
	  失敗時に閉じるセッション名を保持する
	  @param[in] resultName 結果コマンド名
	*/
	explicit RuntimeCloseGuard(const CString& resultName) : mResultName(resultName)
	{
	}
	/**
	  セッションが引き継がれなかった場合に閉じる
	*/
	~RuntimeCloseGuard()
	{
		if (mShouldClose) {
			OutputPreviewRuntime::Get()->Close(mResultName);
		}
	}
	/**
	  セッションの所有権を実行結果へ引き継ぐ
	*/
	void Release() { mShouldClose = false; }

private:
	CString mResultName;
	bool mShouldClose{true};
};

/**
  アプリケーションプロファイル内に一時ファイルのパスを作成する
  @param[in] extension ファイル拡張子
  @param[out] path 生成したファイルパス
  @return パスを生成できた場合はtrue
*/
bool CreateTemporaryPath(const CString& extension, CString& path)
{
	Path profileDir(Path::APPDIRPERMACHINE);
	if (Path::IsDirectory(profileDir) == FALSE && CreateDirectory(profileDir, nullptr) == FALSE &&
		GetLastError() != ERROR_ALREADY_EXISTS) {
		return false;
	}

	Path outputDir(Path::APPDIRPERMACHINE, _T("output-preview"));
	if (Path::IsDirectory(outputDir) == FALSE && CreateDirectory(outputDir, nullptr) == FALSE &&
		GetLastError() != ERROR_ALREADY_EXISTS) {
		return false;
	}

	GUID guid{};
	if (CoCreateGuid(&guid) != S_OK) {
		return false;
	}
	TCHAR guidText[40]{};
	if (StringFromGUID2(guid, guidText, _countof(guidText)) == 0) {
		return false;
	}
	CString fileName(guidText);
	fileName.Remove(_T('{'));
	fileName.Remove(_T('}'));
	fileName.Remove(_T('-'));
	CString normalizedExtension(extension);
	if (normalizedExtension.IsEmpty()) {
		normalizedExtension = _T(".txt");
	}
	else if (normalizedExtension[0] != _T('.')) {
		normalizedExtension.Insert(0, _T('.'));
	}
	fileName += normalizedExtension;
	if (outputDir.Append(fileName) == false) {
		return false;
	}
	path = (LPCTSTR)outputDir;
	return true;
}

/**
  パイプに到着した標準出力を読み取り、必要に応じて保存する
  @param[in] readHandle 読み取りハンドル
  @param[out] output 標準出力の蓄積先
  @param[in] captureOutput 出力を保存するか
*/
void DrainStdout(HANDLE readHandle, std::vector<char>& output, bool captureOutput)
{
	std::vector<char> buffer(65536);
	for (;;) {
		DWORD available = 0;
		if (PeekNamedPipe(readHandle, nullptr, 0, nullptr, &available, nullptr) == FALSE || available == 0) {
			return;
		}
		DWORD readSize = (std::min)(available, static_cast<DWORD>(buffer.size()));
		DWORD readCount = 0;
		if (ReadFile(readHandle, buffer.data(), readSize, &readCount, nullptr) == FALSE || readCount == 0) {
			return;
		}
		if (captureOutput) {
			output.insert(output.end(), buffer.data(), buffer.data() + readCount);
		}
	}
}

/**
  実行対象ファイルと引数から実際に起動するプログラムを解決する
  @param[in] filePath 実行対象ファイル
  @param[in] arguments 実行時引数
  @param[out] executablePath 起動するプログラムのパス
  @param[out] resolvedArguments 起動するプログラムに渡す引数
  @return 解決できた場合はtrue
*/
bool ResolveExecutablePath(
	const CString& filePath,
	const CString& arguments,
	CString& executablePath,
	CString& resolvedArguments
)
{
	CString expandedFilePath(filePath);
	launcherapp::commands::common::ExpandMacros(expandedFilePath);
	launcherapp::utility::LocalPathResolver pathResolver;
	CString resolvedFilePath;
	if (pathResolver.Resolve(expandedFilePath, resolvedFilePath)) {
		expandedFilePath = resolvedFilePath;
	}

	launcherapp::utility::FileAssociationResolver associationResolver;
	return associationResolver.Resolve(expandedFilePath, arguments, executablePath, resolvedArguments);
}

/**
  関連付けを解決したプログラムを起動し、タイムアウトまで標準出力を取得する
  @param[in] param コマンド設定
  @param[in] filePath 実行対象ファイル
  @param[in] arguments 実行時引数
  @param[in] workDir 作業フォルダ
  @param[out] output 標準出力
  @param[out] executablePath 起動したプログラムのパス
  @param[out] errMsg エラーメッセージ
  @return 正常終了した場合はtrue
*/
bool RunProcess(
	const OutputPreviewParam& param,
	const CString& filePath,
	CString& arguments,
	const CString& workDir,
	std::vector<char>& output,
	CString& executablePath,
	String* errMsg
)
{
	CString resolvedArguments;
	if (ResolveExecutablePath(filePath, arguments, executablePath, resolvedArguments) == false) {
		if (errMsg) {
			*errMsg = _T("実行ファイルの関連付けを解決できませんでした");
		}
		return false;
	}

	SECURITY_ATTRIBUTES securityAttributes{};
	securityAttributes.nLength = sizeof(securityAttributes);
	securityAttributes.bInheritHandle = TRUE;
	HANDLE rawReadHandle = nullptr;
	HANDLE rawWriteHandle = nullptr;
	if (CreatePipe(&rawReadHandle, &rawWriteHandle, &securityAttributes, 0) == FALSE) {
		if (errMsg) {
			*errMsg = _T("標準出力を取得するパイプを作成できませんでした");
		}
		return false;
	}
	ScopedHandle readHandle(rawReadHandle);
	ScopedHandle writeHandle(rawWriteHandle);
	SetHandleInformation(readHandle.Get(), HANDLE_FLAG_INHERIT, 0);

	ScopedHandle nullDevice(CreateFile(_T("NUL"), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, &securityAttributes, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
	if (nullDevice.Get() == INVALID_HANDLE_VALUE) {
		if (errMsg) {
			*errMsg = _T("子プロセスの標準入力を準備できませんでした");
		}
		return false;
	}

	STARTUPINFO startupInfo{};
	startupInfo.cb = sizeof(startupInfo);
	startupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startupInfo.hStdInput = nullDevice.Get();
	startupInfo.hStdOutput = writeHandle.Get();
	startupInfo.hStdError = nullDevice.Get();
	startupInfo.wShowWindow = static_cast<WORD>(param.GetShowType());

	CString commandLine = launcherapp::utility::QuoteArgument(executablePath);
	if (resolvedArguments.IsEmpty() == FALSE) {
		commandLine += _T(" ");
		commandLine += resolvedArguments;
	}
	std::vector<TCHAR> commandLineBuffer(commandLine.GetLength() + 1);
	_tcscpy_s(commandLineBuffer.data(), commandLineBuffer.size(), commandLine);

  spdlog::debug(_T("[OutputPreview] RunProcess path:{0} cmdline:{1}"),
                (LPCTSTR)executablePath, (LPCTSTR)commandLineBuffer.data());

	PROCESS_INFORMATION processInfo{};
	LPCTSTR workingDirectory = workDir.IsEmpty() ? nullptr : (LPCTSTR)workDir;
	BOOL started = CreateProcess(executablePath, commandLineBuffer.data(), nullptr, nullptr, TRUE,
		CREATE_UNICODE_ENVIRONMENT, nullptr, workingDirectory, &startupInfo, &processInfo);
	if (started == FALSE) {
		if (errMsg) {
			*errMsg = _T("プログラムを起動できませんでした");
		}
		return false;
	}
	ScopedHandle processHandle(processInfo.hProcess);
	ScopedHandle threadHandle(processInfo.hThread);
	writeHandle.Reset();
	nullDevice.Reset();

	ULONGLONG startTime = GetTickCount64();
	bool timedOut = false;
	bool isShuttingDown = false;
	for (;;) {
		DrainStdout(readHandle.Get(), output, param.mResultType == 0);
		DWORD waitResult = WaitForSingleObject(processHandle.Get(), 10);
		if (waitResult == WAIT_OBJECT_0) {
			DrainStdout(readHandle.Get(), output, param.mResultType == 0);
			break;
		}
		if (waitResult == WAIT_FAILED) {
			TerminateProcess(processHandle.Get(), 1);
			WaitForSingleObject(processHandle.Get(), INFINITE);
			if (errMsg) {
				*errMsg = _T("プロセスの終了状態を確認できませんでした");
			}
			return false;
		}
		if (launcherapp::core::LauncherProcessContext::GetInstance()->IsShutdownInProgress()) {
			isShuttingDown = true;
			TerminateProcess(processHandle.Get(), 1);
			WaitForSingleObject(processHandle.Get(), INFINITE);
			DrainStdout(readHandle.Get(), output, param.mResultType == 0);
			break;
		}
		if (GetTickCount64() - startTime >= static_cast<ULONGLONG>(param.mTimeout)) {
			timedOut = true;
			TerminateProcess(processHandle.Get(), 1);
			WaitForSingleObject(processHandle.Get(), INFINITE);
			DrainStdout(readHandle.Get(), output, param.mResultType == 0);
			break;
		}
	}

	if (isShuttingDown) {
		return false;
	}
	if (timedOut) {
		if (errMsg) {
			*errMsg = _T("タイムアウトしました。実行中のプロセスを終了しました。");
		}
		return false;
	}
	DWORD exitCode = 1;
	if (GetExitCodeProcess(processHandle.Get(), &exitCode) == FALSE ||
		param.ShouldShowResultForExitCode(exitCode) == false) {
		if (errMsg) {
			LastErrorString errStr(GetLastError());
			CString msg;
			msg.Format(_T("実行したプログラムが正常終了しませんでした 終了コード:%d メッセージ:%s"), exitCode, (LPCTSTR)errStr);
			*errMsg = (LPCTSTR)msg;
		}
		return false;
	}
	return true;
}

/**
  出力ファイルを文字列として読み込む
  @param[in] path 読み込むファイル
  @param[out] text 読み込んだ文字列
  @param[in] detectEncoding UTF-8/SJISの判定を行うか
  @return 読み込みに成功した場合はtrue
*/
bool ReadFileAsText(const CString& path, CString& text, bool detectEncoding)
{
	CFile file;
	if (file.Open(path, CFile::modeRead | CFile::typeBinary) == FALSE) {
		return false;
	}
	ULONGLONG length = file.GetLength();
	if (length > static_cast<ULONGLONG>((std::numeric_limits<UINT>::max)())) {
		return false;
	}
	std::vector<char> buffer(static_cast<size_t>(length) + 1, 0);
	size_t offset = 0;
	while (offset < length) {
		UINT readCount = file.Read(buffer.data() + offset, static_cast<UINT>(length - offset));
		if (readCount == 0) {
			return false;
		}
		offset += readCount;
	}
	if (detectEncoding) {
		launcherapp::commands::output_preview::OutputPreviewTextDecoder::Decode(buffer.data(), text);
	}
	else {
		launcherapp::utility::CharConverter converter(CP_UTF8);
		converter.Convert(buffer.data(), text);
	}
	return true;
}

/**
  バイト列を一時ファイルへ書き込む
  @param[in] path 書き込み先
  @param[in] bytes 書き込む内容
  @return 書き込みに成功した場合はtrue
*/
bool WriteBytes(const CString& path, const std::vector<char>& bytes)
{
	std::ofstream file(std::filesystem::path((LPCTSTR)path), std::ios::binary | std::ios::trunc);
	if (file.is_open() == false) {
		return false;
	}
	if (bytes.empty() == false) {
		file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
	}
	return file.good();
}

/**
  プロセスを実行し、結果を共有セッションへ登録してプレビューを表示する
  @param[in] param コマンド設定
  @param[in] actionParam 実行時パラメーター
  @param[out] errMsg エラーメッセージ
  @return 実行と結果表示の準備に成功した場合はtrue
*/
bool ExecuteAndPreview(
	OutputPreviewParam param,
	launcherapp::actions::core::Parameter* actionParam,
	String* errMsg
)
{
	CString resultName = param.mName + _T(" 結果");
	CString inputText = actionParam ? actionParam->GetWholeString() : (LPCTSTR)param.mName;
	auto runtime = OutputPreviewRuntime::Get();
	if (runtime->TryBegin(resultName, param.mName, inputText) == false) {
		if (errMsg) {
			*errMsg = _T("別の実行結果確認コマンドが実行中または表示中です");
		}
		return false;
	}
	RuntimeCloseGuard closeGuard(resultName);

	CString filePath(param.mFilePath);
	launcherapp::commands::common::ExpandMacros(filePath);
	CString resolvedFilePath;
	launcherapp::utility::LocalPathResolver pathResolver;
	if (pathResolver.Resolve(filePath, resolvedFilePath)) {
		filePath = resolvedFilePath;
	}

	CString workingDirectory(param.mWorkDir);
	launcherapp::commands::common::ExpandMacros(workingDirectory);
	launcherapp::commands::common::StripDoubleQuate(workingDirectory);

	CString arguments(param.mParameter);
	launcherapp::commands::common::ExpandArgumentsAndMacros(
		arguments, actionParam, launcherapp::commands::common::ArgumentQuoting::Always, true);

	CString outputFilePath;
	if (param.mResultType == 1) {
		if (CreateTemporaryPath(param.mExtension, outputFilePath) == false ||
			runtime->AddTemporaryFile(resultName, outputFilePath) == false) {
			if (errMsg) {
				*errMsg = _T("出力先の一時ファイルを準備できませんでした");
			}
			return false;
		}
		launcherapp::utility::ReplaceQuotedPlaceholder(arguments, _T("$output"), outputFilePath);
	}

	std::vector<char> output;
	CString executablePath;
	if (RunProcess(param, filePath, arguments, workingDirectory, output, executablePath, errMsg) == false) {
		return false;
	}

	CString resultText;
	CString previewFilePath;
	if (param.mResultType == 0) {
		if (output.empty() == false) {
			std::vector<char> nullTerminated(output);
			nullTerminated.push_back('\0');
			if (param.mPreviewType == 0) {
				launcherapp::commands::output_preview::OutputPreviewTextDecoder::Decode(
					nullTerminated.data(), resultText);
			}
			else {
				launcherapp::utility::CharConverter converter(CP_UTF8);
				converter.Convert(nullTerminated.data(), resultText);
			}
		}
		if (param.mPreviewType == 1) {
			if (CreateTemporaryPath(_T(".html"), previewFilePath) == false ||
				runtime->AddTemporaryFile(resultName, previewFilePath) == false || WriteBytes(previewFilePath, output) == false) {
				if (errMsg) {
					*errMsg = _T("Webコンテンツの一時ファイルを作成できませんでした");
				}
				return false;
			}
		}
	}
	else {
		if (Path::FileExists(outputFilePath) == FALSE ||
			ReadFileAsText(outputFilePath, resultText, param.mPreviewType == 0) == false) {
			if (errMsg) {
				*errMsg = _T("出力ファイルを読み込めませんでした");
			}
			return false;
		}
		if (param.mPreviewType == 1) {
			previewFilePath = outputFilePath;
		}
	}

	if (runtime->Publish(resultName, resultText, previewFilePath, param.mPreviewType, executablePath) == false) {
		if (errMsg) {
			*errMsg = _T("実行結果をプレビューへ渡せませんでした");
		}
		return false;
	}
	closeGuard.Release();
	auto mainWindow = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	mainWindow->ActivateWindow(false);
	mainWindow->SetText(inputText);
	return true;
}

} // 名前空間

struct OutputPreviewCommand::PImpl
{
	/**
	  アイコン用に保持した実行プログラムパスのキャッシュを破棄する
	*/
	void InvalidateIconPathCache()
	{
		mIconPath.Empty();
		mIsIconPathCached = false;
	}

	OutputPreviewParam mParam;
	CString mIconPath;
	bool mIsIconPathCached{false};
};

OutputPreviewCommand::OutputPreviewCommand() : in(std::make_unique<PImpl>())
{
}

OutputPreviewCommand::~OutputPreviewCommand()
{
}

CString OutputPreviewCommand::GetType()
{
	return _T("OutputPreview");
}

CString OutputPreviewCommand::TypeDisplayName()
{
	return _T("実行結果確認コマンド");
}

void OutputPreviewCommand::SetParam(const OutputPreviewParam& param)
{
	in->mParam = param;
	in->InvalidateIconPathCache();
}

const OutputPreviewParam& OutputPreviewCommand::GetParam() const
{
	return in->mParam;
}

bool OutputPreviewCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (UserCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}
	if (ifid == IFID_SELECTIONBEHAVIOR) {
		AddRef();
		*cmd = static_cast<launcherapp::core::SelectionBehavior*>(this);
		return true;
	}
	return false;
}

CString OutputPreviewCommand::GetName()
{
	return in->mParam.mName;
}

CString OutputPreviewCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString OutputPreviewCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool OutputPreviewCommand::CanExecute(String* reasonMsg)
{
	CString path(in->mParam.mFilePath);
	launcherapp::commands::common::ExpandMacros(path);
	if (Path::FileExists(path) == FALSE || Path::IsDirectory(path)) {
		if (reasonMsg) {
			*reasonMsg = _T("実行ファイルが見つかりません");
		}
		return false;
	}
	return true;
}

bool OutputPreviewCommand::IsAcceptArguments()
{
  // 追加の候補リストを表示しない
	return false;
}

bool OutputPreviewCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	CString path(in->mParam.mFilePath);
	launcherapp::commands::common::ExpandMacros(path);
	if (hotkeyAttr.GetModifiers() == MOD_CONTROL) {
		*action = new launcherapp::actions::builtin::OpenPathInFilerAction(path);
		return true;
	}
	if (hotkeyAttr.GetModifiers() == MOD_ALT) {
		*action = new launcherapp::actions::builtin::ShowPropertiesAction(path);
		return true;
	}
	if (hotkeyAttr.GetModifiers() != 0) {
		return false;
	}
	OutputPreviewParam param = in->mParam;
	*action = new launcherapp::actions::builtin::CallbackAction(
		_T("プログラムを実行して結果を表示"), [param](launcherapp::actions::core::Parameter* actionParam, String* errMsg) mutable {
			return ExecuteAndPreview(param, actionParam, errMsg);
		});
	return true;
}

HICON OutputPreviewCommand::GetIcon()
{
	if (in->mIsIconPathCached == false) {
		CString executablePath;
		CString resolvedArguments;
		if (ResolveExecutablePath(in->mParam.mFilePath, _T(""), executablePath, resolvedArguments)) {
			in->mIconPath = executablePath;
		}
		else {
			CString filePath(in->mParam.mFilePath);
			launcherapp::commands::common::ExpandMacros(filePath);
			if (Path::FileExists(filePath)) {
				in->mIconPath = filePath;
			}
		}
		in->mIsIconPathCached = true;
	}
	return in->mIconPath.IsEmpty() ? IconLoader::Get()->LoadPromptIcon() :
		IconLoader::Get()->LoadIconFromPath(in->mIconPath);
}

launcherapp::core::Command* OutputPreviewCommand::Clone()
{
	auto command = new OutputPreviewCommand();
	command->SetParam(in->mParam);
	return command;
}

bool OutputPreviewCommand::Save(CommandEntryIF* entry)
{
	entry->Set(_T("Type"), GetType());
	return in->mParam.Save(entry);
}

bool OutputPreviewCommand::Load(CommandEntryIF* entry)
{
	CString type = entry->Get(_T("Type"), _T(""));
	if (type.IsEmpty() == FALSE && type != GetType()) {
		return false;
	}
	if (in->mParam.Load(entry) == false) {
		return false;
	}
	in->InvalidateIconPathCache();
	return true;
}

bool OutputPreviewCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
{
	if (editor == nullptr) {
		return false;
	}
	auto commandEditor = new OutputPreviewEditor(CWnd::FromHandle(parent));
	commandEditor->SetParam(in->mParam);
	commandEditor->SetOriginalName(GetName());
	*editor = commandEditor;
	return true;
}

bool OutputPreviewCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<OutputPreviewEditor> commandEditor;
	if (editor->QueryInterface(IFID_OUTPUTPREVIEWEDITOR, (void**)&commandEditor) == false) {
		return false;
	}
	SetParam(commandEditor->GetParam());
	return true;
}

bool OutputPreviewCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmd)
{
	RefPtr<OutputPreviewEditor> commandEditor;
	if (editor->QueryInterface(IFID_OUTPUTPREVIEWEDITOR, (void**)&commandEditor) == false) {
		return false;
	}
	auto command = std::make_unique<OutputPreviewCommand>();
	command->SetParam(commandEditor->GetParam());
	if (newCmd) {
		*newCmd = command.release();
	}
	return true;
}

bool OutputPreviewCommand::GetHotKeyAttribute(CommandHotKeyAttribute&)
{
	return false;
}

int OutputPreviewCommand::Match(Pattern* pattern)
{
	CString inputText(pattern->GetWholeString());
	if (OutputPreviewRuntime::Get()->IsResultForCommand(GetName(), inputText)) {
		return Pattern::Mismatch;
	}
	return pattern->Match(GetName());
}

void OutputPreviewCommand::OnSelect(launcherapp::core::Command*)
{
}

void OutputPreviewCommand::OnUnselect(launcherapp::core::Command*)
{
}

launcherapp::core::SelectionBehavior::CloseWindowPolicy OutputPreviewCommand::GetCloseWindowPolicy(uint32_t)
{
	return launcherapp::core::SelectionBehavior::CLOSEWINDOW_ASYNC;
}

bool OutputPreviewCommand::CompleteKeyword(CString&, int&, int&)
{
	return false;
}

}
}
}
