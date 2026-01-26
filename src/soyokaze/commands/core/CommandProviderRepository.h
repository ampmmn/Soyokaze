#pragma once

#include "commands/core/CommandProviderIF.h"
#include <vector>
#include <memory>

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

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}

