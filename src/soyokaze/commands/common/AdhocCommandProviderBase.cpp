#include "pch.h"
#include "AdhocCommandProviderBase.h"
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace common {


AdhocCommandProviderBase::AdhocCommandProviderBase() : mRefCount(1)
{
}

AdhocCommandProviderBase::~AdhocCommandProviderBase()
{
}

// 初回起動の初期化を行う
void AdhocCommandProviderBase::OnFirstBoot()
{
	// 何もしない
}


// コマンドの読み込み
void AdhocCommandProviderBase::LoadCommands(
	CommandFile* cmdFile
)
{
	UNREFERENCED_PARAMETER(cmdFile);

	// 何もしない
}

CString AdhocCommandProviderBase::GetName()
{
	// 派生クラスで実装する必要あり
	assert(0);
	return _T("(Unnamed)");
}

// 作成できるコマンドの種類を表す文字列を取得
CString AdhocCommandProviderBase::GetDisplayName()
{
	// サポートしない
	return _T("");
}

// コマンドの種類の説明を示す文字列を取得
CString AdhocCommandProviderBase::GetDescription()
{
	// サポートしない
	return _T("");
}

// コマンド新規作成ダイアログ
bool AdhocCommandProviderBase::NewDialog(CommandParameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// サポートしない
	return false;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool AdhocCommandProviderBase::IsPrivate() const
{
	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t AdhocCommandProviderBase::AdhocCommandProviderBase::GetOrder() const
{
	return 1500;
}

uint32_t AdhocCommandProviderBase::AdhocCommandProviderBase::AddRef()
{
	return ++mRefCount;
}

uint32_t AdhocCommandProviderBase::Release()
{
	uint32_t n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp


