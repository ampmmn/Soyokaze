#pragma once

#include "commands/core/CommandProviderIF.h"
#include <vector>
#include <memory>
#include <cstddef>

namespace launcherapp { namespace core {

class CommandProvider;

class CommandProviderRepository
{
private:
	CommandProviderRepository();
	~CommandProviderRepository();

public:
	static CommandProviderRepository* GetInstance();

	// コマンドプロバイダ登録
	void Register(CommandProvider* provider);
	// アプリ終了時の処理
	void Finalize();

	// プロバイダを得る
	void EnumProviders(std::vector<CommandProvider*>& providers);
	// 登録されているプロバイダ数を取得する。
	size_t GetProviderCount();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}

