#include "pch.h"
#include "framework.h"
#include "commands/builtin/BuiltinCommandBase.h"
#include "AppPreference.h"
#include "CommandFile.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

BuiltinCommandBase::BuiltinCommandBase(LPCTSTR name) : 
	mName(name? name : _T("")), mRefCount(1)
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
	return pattern->Match(GetName());
}

bool BuiltinCommandBase::IsEditable()
{
	return false;
}

int BuiltinCommandBase::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
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

bool BuiltinCommandBase::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);
	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
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

}
}
}

