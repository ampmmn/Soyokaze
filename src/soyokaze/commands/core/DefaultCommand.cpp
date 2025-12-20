#include "pch.h"
#include "DefaultCommand.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "actions/builtin/RegisterNewCommandAction.h"
#include "actions/builtin/NullAction.h"
#include "actions/core/ActionParameter.h"
#include "core/IFIDDefine.h"
#include "icon/IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace core {

using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;
using SelectionBehavior = launcherapp::core::SelectionBehavior;

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
		mCloseWindowPolicy = pref->GetDefaultActionCloseBehavior();
		mIsLoaded = true;
	}

	CString mName;

	// 実行するアクションを表す種別名
	CString mActionType;
	// 設定読み込み済か?
	bool mIsLoaded{false};
	// コマンド実行時のウインドウクローズ方法
	int mCloseWindowPolicy{SelectionBehavior::CLOSEWINDOW_ASYNC};
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

CString DefaultCommand::GetName()
{
	return _T("");
}

CString DefaultCommand::GetDescription()
{
	if (in->mActionType == _T("register")) {
		RefPtr<ParameterBuilder> paramTmp(ParameterBuilder::Create(in->mName));
		CString text;
		text.Format(_T("\"%s\"を登録"), (LPCTSTR)paramTmp->GetCommandString());
		return text;
	}
	return _T("(一致候補なし)");
}

CString DefaultCommand::GetTypeDisplayName()
{
	return _T("");
}

bool DefaultCommand::CanExecute(String*)
{
	return true;
}

// 修飾キー押下状態に対応した実行アクションを取得する
bool DefaultCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags != 0) {
		return false;
	}

	if (in->mIsLoaded == false) {
		in->Load();
	}

	const auto& type = in->mActionType;
	if (type == _T("copy")) {
		// クリップボードにコピー
		*action = new launcherapp::actions::clipboard::CopyAction();
		return true;
	}
	else if (type == _T("register")) {
		// コマンドを登録
		*action = new launcherapp::actions::builtin::RegisterNewCommandAction(true);
		return true;
	}
	else {
		*action = new launcherapp::actions::builtin::NullAction();
		return true;
	}

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

bool DefaultCommand::IsAllowAutoExecute()
{
	return false;
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

// 選択された
void DefaultCommand::OnSelect(Command* prior)
{
	// 何もしない
	UNREFERENCED_PARAMETER(prior);
}

// 選択解除された
void DefaultCommand::OnUnselect(Command* next)
{
	// 何もしない
	UNREFERENCED_PARAMETER(next);
}

// 実行後のウインドウを閉じる方法
SelectionBehavior::CloseWindowPolicy
DefaultCommand::GetCloseWindowPolicy(uint32_t)
{
	return (SelectionBehavior::CloseWindowPolicy)in->mCloseWindowPolicy;
}

// 選択時に入力欄に設定するキーワードとキャレットを設定する
bool DefaultCommand::CompleteKeyword(CString& , int& , int& )
{
	return false;
}

bool DefaultCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (ifid == IFID_SELECTIONBEHAVIOR) {
		AddRef();
		*cmd = (launcherapp::core::SelectionBehavior*)this;
		return true;
	}
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

