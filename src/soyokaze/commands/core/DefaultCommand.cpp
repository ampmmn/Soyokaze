#include "pch.h"
#include "DefaultCommand.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "commands/common/Clipboard.h"
#include "commands/builtin/NewCommand.h"
#include "icon/IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace core {

using namespace launcherapp::commands::common;
using namespace launcherapp::commands::builtin;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;

struct DefaultCommand::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override	{}
	void OnAppPreferenceUpdated() override
	{
		Load();
	}

	void OnAppExit() override {}

	void Load()
	{
		auto pref = AppPreference::Get();
		mActionType = pref->GetDefaultActionType();
		mIsLoaded = true;
	}

	CString mName;

	// 実行するアクションを表す種別名
	CString mActionType;
	// 設定読み込み済か?
	bool mIsLoaded{false};
};

DefaultCommand::DefaultCommand() : in(new PImpl)
{
	
}

DefaultCommand::~DefaultCommand()
{
}

void DefaultCommand::SetName(const CString& word)
{
	in->mName = word;
}

bool DefaultCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	UNREFERENCED_PARAMETER(ifid);
	UNREFERENCED_PARAMETER(cmd);
	// 未実装
	return false;
}

CString DefaultCommand::GetName()
{
	return _T("");
}

CString DefaultCommand::GetDescription()
{
	if (in->mActionType == _T("register")) {
		CString text;
		text.Format(_T("\"%s\"を登録"), (LPCTSTR)in->mName);
		return text;
	}
	return _T("(一致候補なし)");
}

CString DefaultCommand::GetGuideString()
{
	if (in->mIsLoaded == false) {
		in->Load();
	}

	if (in->mActionType == _T("copy")) {
		return _T("⏎: クリップボードにコピー");
	}
	else if (in->mActionType == _T("register")) {
		return _T("⏎: コマンドを登録");
	}
	return _T("");
}

CString DefaultCommand::GetTypeDisplayName()
{
	return _T("");
}

bool DefaultCommand::CanExecute()
{
	return true;
}

BOOL DefaultCommand::Execute(Parameter* param)
{
	if (in->mIsLoaded == false) {
		in->Load();
	}

	const auto& type = in->mActionType;
	if (type == _T("copy")) {
		// クリップボードにコピー
		auto str = param->GetWholeString();
		Clipboard::Copy(str);
		return TRUE;
	}
	else if (type == _T("register")) {
		// コマンドを登録
		CString str = param->GetWholeString();
		RefPtr<CommandParameterBuilder> commandParam(CommandParameterBuilder::Create(_T("new ") + str), false);

		NewCommand cmd;
		BOOL result = cmd.Execute(commandParam);

		return result;
	}
	return TRUE;
}

CString DefaultCommand::GetErrorString()
{
	return _T("");
}

HICON DefaultCommand::GetIcon()
{
	auto iconLoader = IconLoader::Get();
	const auto& type = in->mActionType;
	if (type == _T("copy")) {
		return iconLoader->LoadCopyIcon();
	}
	else if (type == _T("register")) {
		return iconLoader->LoadNewIcon();
	}
	else {
		return iconLoader->LoadUnknownIcon();
	}
	return nullptr;
}

int DefaultCommand::Match(Pattern* pattern)
{
	UNREFERENCED_PARAMETER(pattern);
	return Pattern::Mismatch;
}

bool DefaultCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	UNREFERENCED_PARAMETER(attr);
	return false;
}

launcherapp::core::Command* DefaultCommand::Clone()
{
	return nullptr;
}

bool DefaultCommand::Save(CommandEntryIF* entry)
{
	UNREFERENCED_PARAMETER(entry);
	// 実装しない
	return false;
}

bool DefaultCommand::Load(CommandEntryIF* entry)
{
	UNREFERENCED_PARAMETER(entry);
	// 実装しない
	return false;
}

uint32_t DefaultCommand::AddRef()
{
	// このコマンドはRepositoryのみが所有権を持つため、参照カウントによる制御を実装しない
	return 1;
}

uint32_t DefaultCommand::Release()
{
	return 1;
}


}
}
}

