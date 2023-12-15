#include "pch.h"
#include "VMXFileCommandProvider.h"
#include "commands/vmware/VMXFileCommand.h"
#include "core/CommandRepository.h"
#include <vector>
#include <regex>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace vmware {

using CommandRepository = soyokaze::core::CommandRepository;

struct VMXFileCommandProvider::PImpl
{
	PImpl()
	{
	}
	virtual ~PImpl()
	{
		for (auto& cmd : mCommands) {
			cmd->Release();
		}
	}

	void Reload();

	CString mPrefFilePath;
	FILETIME mPrefUpdateTime;

	std::vector<VMXFileCommand*> mCommands;
};

/**
	preference.iniを読み、.vmxファイルのMRU一覧を生成する
*/
void VMXFileCommandProvider::PImpl::Reload()
{
	for (auto& cmd : mCommands) {
		cmd->Release();
	}
	mCommands.clear();

	FILE* fpIn = nullptr;
	if (_tfopen_s(&fpIn, mPrefFilePath, _T("r")) != 0) {
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

	for (auto& entry : items) {
		auto item = entry.second;
		if (PathFileExists(item.filePath) == FALSE) {
			// 存在しない.vmxファイルは除外する
			continue;
		}
		mCommands.push_back(new VMXFileCommand(item.displayName, item.filePath));
	}

	file.Close();
	fclose(fpIn);

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(VMXFileCommandProvider)


VMXFileCommandProvider::VMXFileCommandProvider() : in(std::make_unique<PImpl>())
{
	size_t reqLen = 0;
	LPTSTR p = in->mPrefFilePath.GetBuffer(MAX_PATH_NTFS);
	_tgetenv_s(&reqLen, p, MAX_PATH_NTFS, _T("APPDATA"));
	PathAppend(p, _T("VMWare/preferences.ini"));
	memset(&in->mPrefUpdateTime,0 , sizeof(in->mPrefUpdateTime));
	in->mPrefFilePath.ReleaseBuffer();
}

VMXFileCommandProvider::~VMXFileCommandProvider()
{
}

CString VMXFileCommandProvider::GetName()
{
	return _T("VMXFile");
}

static bool GetLastUpdateTime(LPCTSTR path, FILETIME& ftime)
{
	HANDLE h = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}
	GetFileTime(h, nullptr, nullptr, &ftime);
	CloseHandle(h);
	return true;
}


// 一時的なコマンドを必要に応じて提供する
void VMXFileCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	FILETIME lastUpdate;
	if (GetLastUpdateTime(in->mPrefFilePath, lastUpdate) == false) {
		return;
	}
	if (memcmp(&in->mPrefUpdateTime, &lastUpdate, sizeof(FILETIME)) != 0) {
		// 前回から更新されたためファイルを再読み込みする

		in->Reload();
	}
	in->mPrefUpdateTime = lastUpdate;

	for (auto& cmd : in->mCommands) {

		int level = cmd->Match(pattern);
		if (level == Pattern::Mismatch) {
			continue;
		}

		cmd->AddRef();
		commands.push_back(CommandQueryItem(level, cmd));
	}
}


} // end of namespace vmware
} // end of namespace commands
} // end of namespace soyokaze

