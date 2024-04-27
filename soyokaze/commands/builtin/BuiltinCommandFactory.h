#pragma once

#include <memory>
#include <functional>
#include "commands/core/CommandIF.h"
#include "commands/core/CommandFile.h"

namespace launcherapp {
namespace commands {
namespace builtin {

class BuiltinCommandFactory
{
public:
	using Entry = CommandFile::Entry;
	using FactoryMethodType = std::function<launcherapp::core::Command*(Entry*)>;
	using CommandType = launcherapp::core::Command;

private:
	BuiltinCommandFactory();
	~BuiltinCommandFactory();

public:
	static BuiltinCommandFactory* GetInstance();

	void Register(LPCTSTR typeName,FactoryMethodType func); 
	void EnumTypeName(std::vector<CString>& typeNames);
	bool Create(LPCTSTR typeName, Entry* entry, CommandType** cmd);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}  // end of launcherapp
}  // end of commands
}  // end of builtin

// 組み込みコマンド派生クラス側で下記のマクロを通じて組み込みコマンドとして登録する

#define DECLARE_BUILTINCOMMAND(clsName)   static bool RegisterAsBuiltinCommand(); \
	private: \
	static bool _mIsRegistered; \
	public: \
	static bool IsRegistered(); \
	static launcherapp::core::Command* Create(Entry* entry) { \
		auto cmd = new clsName(entry ? (LPCTSTR)CommandFile::GetName(entry) : nullptr); \
		if (entry) { cmd->LoadFrom(entry); } \
		return cmd; \
	}


#define REGISTER_BUILTINCOMMAND(clsName)   \
	bool clsName::RegisterAsBuiltinCommand() { \
		try { \
			launcherapp::commands::builtin::BuiltinCommandFactory::GetInstance()->Register(clsName::TYPE, clsName::Create); \
			return true; \
		} catch(...) { return false; } \
	} \
	bool clsName::_mIsRegistered = clsName::RegisterAsBuiltinCommand(); \
	bool clsName::IsRegistered() { return _mIsRegistered; }


