#include "pch.h"
#include "AfxWWrapper.h"
#include "commands/share/AfxWFunctions.h"

#pragma warning( push )
#pragma warning( disable : 26495 26437 26450 26498 26800 6285 6385)
#include "spdlog/spdlog.h"
#pragma warning( pop )

#include "utility/DemotedProcessToken.h"  // IsRunningAsAdmin
#include "commands/common/NormalPriviledgeProcessProxy.h"

using NormalPriviledgeProcessProxy = launcherapp::commands::common::NormalPriviledgeProcessProxy;

struct AfxWWrapper::PImpl
{
	bool GetCurrentDirDirectly(std::wstring& curDir);
	bool GetCurrentDirViaProxy(std::wstring& curDir);
	bool SetCurrentDirDirectly(const std::wstring& curDir);
	bool SetCurrentDirViaProxy(const std::wstring& curDir);
};

bool AfxWWrapper::PImpl::GetCurrentDirDirectly(std::wstring& curDir)
{
	return AfxW_GetCurrentDir(curDir);
}

bool AfxWWrapper::PImpl::GetCurrentDirViaProxy(std::wstring& path)
{
	auto proxy = NormalPriviledgeProcessProxy::GetInstance();
	return proxy->GetCurrentAfxwDir(path);
}

bool AfxWWrapper::PImpl::SetCurrentDirDirectly(const std::wstring& path)
{
	return AfxW_SetCurrentDir(path);
}

bool AfxWWrapper::PImpl::SetCurrentDirViaProxy(const std::wstring& path)
{
	auto proxy = NormalPriviledgeProcessProxy::GetInstance();
	return proxy->SetCurrentAfxwDir(path);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// コンストラクタ
AfxWWrapper::AfxWWrapper() : in(std::make_unique<PImpl>())
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
	if (DemotedProcessToken::IsRunningAsAdmin() == false) {
		return in->GetCurrentDirDirectly(curDir);
	}
	else {
		return in->GetCurrentDirViaProxy(curDir);
	}
}

// 自窓のカレントディレクトリを移動
bool AfxWWrapper::SetCurrentDir(const std::wstring& path)
{
	if (DemotedProcessToken::IsRunningAsAdmin() == false) {
		return in->SetCurrentDirDirectly(path);
	}
	else {
		return in->SetCurrentDirViaProxy(path);
	}
}


