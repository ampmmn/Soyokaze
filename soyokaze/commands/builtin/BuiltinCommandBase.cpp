#include "pch.h"
#include "framework.h"
#include "commands/builtin/BuiltinCommandBase.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "commands/core/CommandRepository.h"
#include "commands/builtin/BuiltinEditDialog.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

BuiltinCommandBase::BuiltinCommandBase(LPCTSTR name) : 
	mName(name? name : _T("")), mRefCount(1)
{
}

BuiltinCommandBase::BuiltinCommandBase(const BuiltinCommandBase& rhs) :
	mName(rhs.mName),
	mDescription(rhs.mDescription),
	mError(rhs.mError),
	mRefCount(1),
	mIsConfirmBeforeRun(rhs.mIsConfirmBeforeRun),
	mCanSetConfirm(rhs.mCanSetConfirm),
	mIsEnable(rhs.mIsEnable),
	mCanDisable(rhs.mCanDisable)
{
}

BuiltinCommandBase::~BuiltinCommandBase()
{
}

CString BuiltinCommandBase::GetName()
{
	return mName;
}

CString BuiltinCommandBase::GetDescription()
{
	return mDescription;
}

CString BuiltinCommandBase::GetGuideString()
{
	return _T("Enter:実行");
}

CString BuiltinCommandBase::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_BUILTIN);
	return TEXT_TYPE;
}

CString BuiltinCommandBase::GetErrorString()
{
	return mError;
}

HICON BuiltinCommandBase::GetIcon()
{
	return IconLoader::Get()->LoadDefaultIcon();
}

int BuiltinCommandBase::Match(Pattern* pattern)
{
	if (mIsEnable == false) {
		return Pattern::Mismatch;
	}
	return pattern->Match(GetName());
}

bool BuiltinCommandBase::IsEditable()
{
	return (mCanDisable || mCanSetConfirm);
}

bool BuiltinCommandBase::IsDeletable()
{
	// 組み込みコマンドは削除させない
	return false;
}

int BuiltinCommandBase::EditDialog(const Parameter* param)
{
	if (mCanDisable == false && mCanSetConfirm == false) {
		return -1;
	}

	BuiltinEditDialog dlg(GetName(), GetDescription(), mCanDisable, mCanSetConfirm);
	dlg.SetEnable(mIsEnable);
	dlg.SetConfirm(mIsConfirmBeforeRun);

	if (dlg.DoModal() != IDOK) {
		return -1;
	}

	mName = dlg.GetName();
	mIsEnable = dlg.GetEnable();
	mIsConfirmBeforeRun = dlg.GetConfirm();

	// 名前の変更を登録しなおす
	auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
	cmdRepo->ReregisterCommand(this);

	return 0;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @remark 同一のコマンド種別の間で表示順序を維持したいようなケースで重みづけを使わないようにできる
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool BuiltinCommandBase::IsPriorityRankEnabled()
{
	// 基本は重みづけをする
	return true;
}

bool BuiltinCommandBase::Save(CommandEntryIF* entry)
{
	ASSERT(entry);
	entry->Set(_T("Type"), GetType());

	entry->Set(_T("IsConfirmBeforeRun"), mIsConfirmBeforeRun);
	entry->Set(_T("IsEnable"), mIsEnable);

	return true;
}

bool BuiltinCommandBase::Load(CommandEntryIF* entry)
{
	ASSERT(entry);
	mIsConfirmBeforeRun = entry->Get(_T("IsConfirmBeforeRun"), false);
	mIsEnable = entry->Get(_T("IsEnable"), false);
	return true;
}

uint32_t BuiltinCommandBase::AddRef()
{
	return ++mRefCount;
}

uint32_t BuiltinCommandBase::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

void BuiltinCommandBase::LoadFrom(Entry* entry)
{
	Load(entry);
}

}
}
}

