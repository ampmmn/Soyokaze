#include "pch.h"
#include "StartupParam.h"
#include "app/Arguments.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct StartupParam::PImpl
{
public:
	PImpl(int argc, TCHAR* argv[]) : mArgs(argc, argv)
	{
	}

	Arguments mArgs;
};


StartupParam::StartupParam(int argc, TCHAR* argv[]) : 
	in(std::make_unique<PImpl>(argc, argv))
{
}

StartupParam::~StartupParam()
{
}

bool StartupParam::HasRunCommand(CString& commands)
{
	return in->mArgs.GetBWOptValue(_T("/Runcommand="), commands) ||
	       in->mArgs.GetValue(_T("-c"), commands);
}

// 引数のうち -c ... または /Runcommand=... の部分を一つぶんカットする
void StartupParam::ShiftRunCommand()
{
	LPCTSTR bwOptName = _T("/Runcommand=");
	int len = (int)_tcslen(bwOptName);

	int numArgs = in->mArgs.GetCount();
	for (int i = 0; i < numArgs; ++i) {
		auto arg = in->mArgs.Get(i);

		// -cと後続の値をカットする
		if (arg == _T("-c") && i + 1 < numArgs) {
			in->mArgs.Erase(i);
			in->mArgs.Erase(i);
			return;
		}
		// /RunCommand=と後続の値をカットする
		auto argPart = arg.Left(len);
		if (_tcsicmp(bwOptName, argPart) == 0) {
			in->mArgs.Erase(i);
			return;
		}
	}
}


bool StartupParam::HasPathToRegister(CString& pathToRegister)
{
	// 有効な(存在している)パスが指定された場合は登録すべきパスとして扱う
	if (in->mArgs.GetCount() > 1 && PathFileExists(in->mArgs.Get(1))) {
		pathToRegister = in->mArgs.Get(1);
		return true;
	}
	return false;
}

bool StartupParam::HasHideOption()
{
	return in->mArgs.Has(_T("/Hide"));
}

//
bool StartupParam::HasPasteOption(CString& value)
{
	return in->mArgs.GetValue(_T("/Paste"), value) ||
	       in->mArgs.GetBWOptValue(_T("/Paste="), value);
}

bool StartupParam::GetSelectRange(int& startPos, int& selLength)
{
	if (in->mArgs.Has(_T("/SelStart")) == false && in->mArgs.Has(_T("/SelLength")) == false) {
		SPDLOG_DEBUG(_T("range is not specified."));
		return false;
	}

	startPos = 0;

	CString value;
	if (in->mArgs.GetBWOptValue(_T("/SelStart="), value)) {
		startPos = _ttoi(value);
	}

	if (startPos < -1) {
		// startPos=-1を選択解除として扱う
		spdlog::warn(_T("startPos is out of bounds. {}"), startPos);
		startPos = -1;
	}

	selLength = 0;
	if (in->mArgs.GetBWOptValue(_T("/SelLength="), value)) {
		selLength = _ttoi(value);
	}
	return true;
}

// ディレクトリ変更するオプションが指定されたか?
bool StartupParam::HasChangeDirectoryOption(CString& dirPath)
{
	return in->mArgs.GetValue(_T("/ChangeDir"), dirPath) ||
	       in->mArgs.GetBWOptValue(_T("/ChangeDir="), dirPath);

}

