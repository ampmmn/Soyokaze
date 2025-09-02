#include "pch.h"
#include "StartupParam.h"
#include "app/Arguments.h"
#include "utility/Path.h"

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

bool StartupParam::HasRunCommand(String& commands)
{
	if (in->mArgs.GetBWOptValue("/Runcommand=", commands)) {
		return true;
	}
	if (in->mArgs.GetValue("-c", commands)) {
		return true;
	}
	return false;
}

// 引数のうち -c ... または /Runcommand=... の部分を一つぶんカットする
void StartupParam::ShiftRunCommand()
{
	const char* bwOptName = "/Runcommand=";
	int len = (int)strlen(bwOptName);

	int numArgs = in->mArgs.GetCount();
	for (int i = 0; i < numArgs; ++i) {
		auto arg = in->mArgs.Get(i);

		// -cと後続の値をカットする
		if (arg == "-c" && i + 1 < numArgs) {
			in->mArgs.Erase(i);
			in->mArgs.Erase(i);
			return;
		}
		// /RunCommand=と後続の値をカットする
		String argPart = arg.Left(len);
		if (argPart.CompareNoCase(bwOptName) == 0) {
			in->mArgs.Erase(i);
			return;
		}
	}
}


bool StartupParam::HasPathToRegister(String& pathToRegister)
{
	// 有効な(存在している)パスが指定された場合は登録すべきパスとして扱う
	if (in->mArgs.GetCount() > 1 && Path::FileExists(in->mArgs.Get(1).c_str())) {
		pathToRegister = in->mArgs.Get(1);
		return true;
	}
	return false;
}

bool StartupParam::HasHideOption()
{
	return in->mArgs.Has("/Hide");
}

//
bool StartupParam::HasPasteOption(String& value)
{
	if (in->mArgs.GetValue("/Paste", value)) {
		return true;
	}
	if (in->mArgs.GetBWOptValue("/Paste=", value)) {
		return true;
	}
	return false;
}

bool StartupParam::GetSelectRange(int& startPos, int& selLength)
{
	if (in->mArgs.Has("/SelStart") == false && in->mArgs.Has("/SelLength") == false) {
		SPDLOG_DEBUG("range is not specified.");
		return false;
	}

	startPos = 0;

	String value;
	if (in->mArgs.GetBWOptValue("/SelStart=", value)) {
		startPos = std::stoi(value);
	}

	if (startPos < -1) {
		// startPos=-1を選択解除として扱う
		spdlog::warn(_T("startPos is out of bounds. {}"), startPos);
		startPos = -1;
	}

	selLength = 0;
	if (in->mArgs.GetBWOptValue("/SelLength=", value)) {
		selLength = std::stoi(value);
	}
	return true;
}

// ディレクトリ変更するオプションが指定されたか?
bool StartupParam::HasChangeDirectoryOption(String& dirPath)
{
	if (in->mArgs.GetValue("/ChangeDir", dirPath)) {
		return true;
	}
	if (in->mArgs.GetBWOptValue("/ChangeDir=", dirPath)) {
		return true;
	}
	return false;
}

