#pragma once

#include <memory>
#include <string>

namespace launcherproxy {

class ProxyCommand;

class ProxyCommandRepository
{
	ProxyCommandRepository();
	~ProxyCommandRepository();

public:
	static ProxyCommandRepository* GetInstance();

	bool Register(ProxyCommand* command);
	ProxyCommand* GetCommand(const std::string& name);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace launcherproxy

// ProxyCommand派生クラス側で下記のマクロを通じてRepositoryに登録する
#define DECLARE_PROXYCOMMAND(clsName)   static bool RegisterProxyCommand(); \
	private: \
	static bool _mIsProxyCommandRegistered; \
	public: \
	static bool IsProxyCommandRegistered();

#define REGISTER_PROXYCOMMAND(clsName)   \
	bool clsName::RegisterProxyCommand() { \
		try { \
			clsName* inst = new clsName(); \
			launcherproxy::ProxyCommandRepository::GetInstance()->Register(inst); \
			return true; \
		} catch(...) { return false; } \
	} \
	bool clsName::_mIsProxyCommandRegistered = clsName::RegisterProxyCommand(); \
	bool clsName::IsProxyCommandRegistered() { return _mIsProxyCommandRegistered; }





