// あ
#include "pch.h"
#include "BuiltinCommandFactory.h"
#include <map>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

using FactoryMethodType = BuiltinCommandFactory::FactoryMethodType;

struct BuiltinCommandFactory::PImpl
{
	std::map<CString, FactoryMethodType> mFactoryMap;
};

BuiltinCommandFactory::BuiltinCommandFactory() : in(new PImpl)
{
}

BuiltinCommandFactory::~BuiltinCommandFactory()
{
}

BuiltinCommandFactory* BuiltinCommandFactory::GetInstance()
{
	static BuiltinCommandFactory inst;
	return &inst;
}

void BuiltinCommandFactory::Register(LPCTSTR typeName, FactoryMethodType func)
{
	ASSERT(typeName != nullptr);
	in->mFactoryMap[typeName] = func;
}

// 登録済のタイプ名一覧を取得
void BuiltinCommandFactory::EnumTypeName(std::vector<CString>& typeNames)
{
	typeNames.reserve(in->mFactoryMap.size());
	for (auto& item : in->mFactoryMap) {
		typeNames.push_back(item.first);
	}
}

bool BuiltinCommandFactory::Create(LPCTSTR typeName, Entry* entry, CommandType** cmd)
{
	ASSERT(cmd);

	auto it = in->mFactoryMap.find(typeName);
	if (it == in->mFactoryMap.end()) {
		SPDLOG_ERROR(_T("command type {0} is not registered."), typeName);
		return false;
	}

	*cmd = it->second(entry);
	return true;
}


}  // end of launcherapp
}  // end of commands
}  // end of builtin


