#include "pch.h"
#include "CommandProviderRepository.h"
#include <mutex>

namespace launcherapp { namespace core {

struct CommandProviderRepository::PImpl
{
	std::vector<CommandProvider*> mProviders;
	std::mutex mMutex;
};

CommandProviderRepository::CommandProviderRepository() : in(new PImpl)
{
}

CommandProviderRepository::~CommandProviderRepository()
{
}

CommandProviderRepository* CommandProviderRepository::GetInstance()
{
	static CommandProviderRepository inst;
	return &inst;
}

// コマンドプロバイダ登録
void CommandProviderRepository::Register(CommandProvider* provider)
{
	std::lock_guard<std::mutex> lock(in->mMutex);

	provider->AddRef();
	in->mProviders.push_back(provider);

	// オーダー順に並び替える
	std::sort(in->mProviders.begin(), in->mProviders.end(),
	          [](const CommandProvider* l, const CommandProvider* r) { return l->GetOrder() < r->GetOrder(); });
}

void CommandProviderRepository::Finalize()
{
	std::lock_guard<std::mutex> lock(in->mMutex);

	for (auto& provider : in->mProviders) {
		provider->Release();
	}
	in->mProviders.clear();
}

// プロバイダを得る
void CommandProviderRepository::EnumProviders(std::vector<CommandProvider*>& providers)
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	providers.clear();
	providers = in->mProviders;
}


}}


