#include "pch.h"
#include "MailToCommandProvider.h"
#include "commands/mailto/MailToCommand.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace soyokaze {
namespace commands {
namespace mailto {


using CommandRepository = soyokaze::core::CommandRepository;

struct MailToCommandProvider::PImpl
{
	uint32_t mRefCount;

	MailToCommand* mCommandPtr;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(MailToCommandProvider)


MailToCommandProvider::MailToCommandProvider() : in(new PImpl)
{
	in->mRefCount = 1;
	in->mCommandPtr = new MailToCommand();
}

MailToCommandProvider::~MailToCommandProvider()
{
	if (in->mCommandPtr) {
		in->mCommandPtr->Release();
	}
}

// 初回起動の初期化を行う
void MailToCommandProvider::OnFirstBoot()
{
	// 何もしない
}


// コマンドの読み込み
void MailToCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	// サポートしない
}

CString MailToCommandProvider::GetName()
{
	return _T("MailToCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString MailToCommandProvider::GetDisplayName()
{
	// サポートしない
	return _T("");
}

// コマンドの種類の説明を示す文字列を取得
CString MailToCommandProvider::GetDescription()
{
	// サポートしない
	return _T("");
}

// コマンド新規作成ダイアログ
bool MailToCommandProvider::NewDialog(const CommandParameter* param)
{
	// サポートしない
	return false;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool MailToCommandProvider::IsPrivate() const
{
	return true;
}

// 一時的なコマンドを必要に応じて提供する
void MailToCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	std::vector<CommandQueryItem>& commands
)
{
	int level = in->mCommandPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		in->mCommandPtr->AddRef();
		commands.push_back(CommandQueryItem(level, in->mCommandPtr));
	}
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t MailToCommandProvider::MailToCommandProvider::GetOrder() const
{
	return 1000;
}

uint32_t MailToCommandProvider::MailToCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t MailToCommandProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace mailto
} // end of namespace commands
} // end of namespace soyokaze

