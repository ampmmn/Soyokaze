#include "pch.h"
#include "AfxWWrapper.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"

using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// コンストラクタ
AfxWWrapper::AfxWWrapper()
{
}

// デストラクタ
AfxWWrapper::~AfxWWrapper()
{
}

/**
  自窓のディレクトリパスを取得
 	@param curDir 自窓のディレクトリパス
*/
bool AfxWWrapper::GetCurrentDir(std::wstring& curDir)
{
	// 管理者権限でアプリを実行していると、通常権限で実行しているあふwのAPI呼び出しに失敗する
	// launcher_proxy.exeを経由することにより、常に通常権限で実行されるようにする。
	auto proxy = NormalPriviledgeProcessProxy::GetInstance();
	return proxy->GetCurrentAfxwDir(curDir);
}

// 自窓のカレントディレクトリを移動
bool AfxWWrapper::SetCurrentDir(const std::wstring& path)
{
	// 管理者権限でアプリを実行していると、通常権限で実行しているあふwのAPI呼び出しに失敗する
	// launcher_proxy.exeを経由することにより、常に通常権限で実行されるようにする。
	auto proxy = NormalPriviledgeProcessProxy::GetInstance();
	return proxy->SetCurrentAfxwDir(path);
}

// あふの自窓の選択ファイルパスを取得
bool AfxWWrapper::GetSelectionPath(std::wstring& path, int index)
{
	auto proxy = NormalPriviledgeProcessProxy::GetInstance();
	return proxy->GetAfxSelectionPath(path, index);
}


