#include "pch.h"
#include "PyExtensionCommandProvider.h"
#include "commands/py_extension/PyExtensionCommand.h"
#include "commands/core/CommandRepository.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace py_extension {


using CommandRepository = launcherapp::core::CommandRepository;

struct PyExtensionCommandProvider::PImpl
{
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(PyExtensionCommandProvider)

IMPLEMENT_LOADFROM(PyExtensionCommandProvider, PyExtensionCommand)

PyExtensionCommandProvider::PyExtensionCommandProvider() : in(std::make_unique<PImpl>())
{
}

PyExtensionCommandProvider::~PyExtensionCommandProvider()
{
}

CString PyExtensionCommandProvider::GetName()
{
	return _T("PyExtension");
}

CString PyExtensionCommandProvider::GetDisplayName()
{
	return _T("Python拡張コマンド");
}

// コマンドの種類の説明を示す文字列を取得
CString PyExtensionCommandProvider::GetDescription()
{
	return _T("Pythonで機能を定義するコマンドです。\n");
}

// コマンド新規作成ダイアログ
bool PyExtensionCommandProvider::NewDialog(Parameter* param)
{
	PyExtensionCommand* newCmd{nullptr};
	if (PyExtensionCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	CommandRepository::GetInstance()->RegisterCommand(newCmd);
	return true;
}


// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t PyExtensionCommandProvider::GetOrder() const
{
	return 6000;
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t PyExtensionCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(PyExtensionCommand::TypeDisplayName());
	return 1;
}


}}} // end of namespace launcherapp::commands::py_extension

