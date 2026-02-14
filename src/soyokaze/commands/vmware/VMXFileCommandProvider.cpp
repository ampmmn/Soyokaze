#include "pch.h"
#include "VMXFileCommandProvider.h"
#include "commands/vmware/VMXFileCommand.h"
#include "commands/core/CommandRepository.h"
#include "utility/Path.h"
#include "utility/LocalDirectoryWatcher.h"
#include "utility/SHA1.h"
#include <vector>
#include <regex>
#include <mutex>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace vmware {

using CommandRepository = launcherapp::core::CommandRepository;

struct VMXFileCommandProvider::PImpl
{
	PImpl()
	{
	}
	virtual ~PImpl()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		for (auto& cmd : mCommands) {
			cmd->Release();
		}
	}

	void Reload();

	CString mPrefFilePath;
	CString mSHA1;

	std::vector<VMXFileCommand*> mCommands;
	std::mutex mMutex;
};

static CString GetSHA1(const CString& filePath)
{
	HANDLE hFile = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr,
	                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE) {
		spdlog::error(_T("Failed to open file {}"), (LPCTSTR)filePath);
		return _T("");
	}

	LARGE_INTEGER fileSize;
	GetFileSizeEx(hFile, &fileSize);

	std::vector<uint8_t> data;
	data.resize(static_cast<size_t>(fileSize.QuadPart));

	DWORD bytesRead = 0;
	BOOL result = ReadFile(hFile, data.data(), static_cast<DWORD>(data.size()), &bytesRead, nullptr);
	if (result == FALSE) {
		spdlog::warn(_T("GetSHA1: Failed to read file {0}. error code: {1}"), (LPCTSTR)filePath, GetLastError());
	}

	CloseHandle(hFile);

	SHA1 sha1;
	sha1.Add(data);

	return sha1.Finish();
}

/**
	preference.iniを読み、.vmxファイルのMRU一覧を生成する
*/
void VMXFileCommandProvider::PImpl::Reload()
{
	Path tmpPath(Path::APPDIRPERMACHINE, _T("preference.ini"));
	CString tmpPathStr(tmpPath);
	CopyFile(mPrefFilePath, tmpPathStr, FALSE);

	auto sha1 = GetSHA1(tmpPathStr);
	if (sha1 == mSHA1) {
		// ファイル内容に変化がなければ読み込まない
	 	return;
	}
	mSHA1 = sha1;

	std::thread th([&, tmpPathStr]() {

		FILE* fpIn = nullptr;
		if (_tfopen_s(&fpIn, tmpPathStr, _T("r")) != 0 || fpIn == nullptr) {
			return;
		}

		tregex regFileName(_T("^pref\\.mruVM(\\d+)\\.filename *= *\"(.+)\"$"));
		tregex regDisplayName(_T("^pref\\.mruVM(\\d+)\\.displayName *= *\"(.+)\"$"));

		struct ITEM {
			CString filePath;
			CString displayName;
		};

		std::map<int, ITEM> items;

		// ファイルを読む
		CStdioFile file(fpIn);
		CString strLine;
		while(file.ReadString(strLine)) {

			strLine.Trim();
			if (strLine.IsEmpty()) {
				continue;
			}

			std::wstring pat(strLine);
			if (std::regex_match(pat, regFileName)) {
				tstring indexStr = std::regex_replace(pat, regFileName,  _T("$1"));
				tstring filePath = std::regex_replace(pat, regFileName,  _T("$2"));
				items[_ttoi(indexStr.c_str())].filePath = filePath.c_str();
			}
			else if (std::regex_match(pat, regDisplayName)) {
				tstring indexStr = std::regex_replace(pat, regDisplayName,  _T("$1"));
				tstring displayName = std::regex_replace(pat, regDisplayName,  _T("$2"));
				items[_ttoi(indexStr.c_str())].displayName = displayName.c_str();
			}
		}
		file.Close();
		fclose(fpIn);
		DeleteFile(tmpPathStr);

		// ロードしたファイルの内容に基づき、コマンドを生成する
		std::vector<VMXFileCommand*> commands;
		for (auto& entry : items) {
			auto item = entry.second;
			if (Path::FileExists(item.filePath) == FALSE) {
				// 存在しない.vmxファイルは除外する
				continue;
			}
			commands.push_back(new VMXFileCommand(item.displayName, item.filePath));
		}

		// 前回のコマンドと今回ロードしたコマンドを入れ替える
		std::lock_guard<std::mutex> lock(mMutex);
		mCommands.swap(commands);

		// 前回のコマンドは不要なので解放する
		for (auto& cmd : commands) {
			cmd->Release();
		}
	});
	th.detach();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(VMXFileCommandProvider)


VMXFileCommandProvider::VMXFileCommandProvider() : in(std::make_unique<PImpl>())
{
	Path path(Path::APPDATA, _T("VMWare\\preferences.ini"));
	in->mPrefFilePath = (LPCTSTR)path;
}

VMXFileCommandProvider::~VMXFileCommandProvider()
{
}

CString VMXFileCommandProvider::GetName()
{
	return _T("VMXFile");
}

// 一時的なコマンドの準備を行うための初期化
void VMXFileCommandProvider::PrepareAdhocCommands()
{
	// ファイルをロードしてVMXFileCommandを生成
	in->Reload();
	
	// ファイルが更新されたら通知を受け取るための登録をする
	LocalDirectoryWatcher::GetInstance()->Register(in->mPrefFilePath, [](void* p) {

			// 更新通知をうけてすぐにpreference.iniにアクセスすると、
			// vmxファイルを開いたときにエラーがでることがあったのですこし遅延を入れてみる
			Sleep(3000);

			// ファイルをロードしてVMXFileCommandを生成
			auto thisPtr = (VMXFileCommandProvider*)p;
			thisPtr->in->Reload();
	}, this);
}

// 一時的なコマンドを必要に応じて提供する
void VMXFileCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	for (auto& cmd : in->mCommands) {

		int level = cmd->Match(pattern);
		if (level == Pattern::Mismatch) {
			continue;
		}

		cmd->AddRef();
		commands.Add(CommandQueryItem(level, cmd));
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t VMXFileCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(VMXFileCommand::TypeDisplayName());
	return 1;
}


} // end of namespace vmware
} // end of namespace commands
} // end of namespace launcherapp

