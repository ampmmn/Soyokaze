#include "pch.h"
#include "ErrorIndicatorCommand.h"
#include "commands/core/CommandRepository.h"
#include "icon/IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using Command = launcherapp::core::Command;

namespace launcherapp { namespace commands { namespace error {

struct ErrorIndicatorCommand::PImpl
{
	RefPtr<Command> mTarget;
	uint32_t mRefCount{1};
};

ErrorIndicatorCommand::ErrorIndicatorCommand() : in(new PImpl)
{
	
}

ErrorIndicatorCommand::~ErrorIndicatorCommand()
{
}

void ErrorIndicatorCommand::SetTarget(Command* cmd)
{
	in->mTarget.reset(cmd);
	in->mTarget->AddRef();
}

bool ErrorIndicatorCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	UNREFERENCED_PARAMETER(ifid);
	UNREFERENCED_PARAMETER(cmd);
	// 未実装
	return false;
}

CString ErrorIndicatorCommand::GetName()
{
	return in->mTarget->GetName();
}

CString ErrorIndicatorCommand::GetDescription()
{
	return in->mTarget->GetErrorString();
}

CString ErrorIndicatorCommand::GetGuideString()
{
	if (in->mTarget.get() == nullptr) {
		return _T("");
	}
	return _T("⏎: コマンドを編集");
}

CString ErrorIndicatorCommand::GetTypeDisplayName()
{
	return in->mTarget->GetTypeDisplayName();
}

bool ErrorIndicatorCommand::CanExecute()
{
	return false;
}

BOOL ErrorIndicatorCommand::Execute(Parameter* param)
{
	if (in->mTarget.get() == nullptr) {
		return TRUE;
	}

	// 編集
	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	constexpr bool isClone = false;
	cmdRepoPtr->EditCommandDialog(in->mTarget->GetName(), isClone);

	return TRUE;
}

CString ErrorIndicatorCommand::GetErrorString()
{
	return _T("");
}

HICON ErrorIndicatorCommand::GetIcon()
{
	return IconLoader::Get()->GetShell32Icon(-16777);
}

int ErrorIndicatorCommand::Match(Pattern* pattern)
{
	UNREFERENCED_PARAMETER(pattern);
	return Pattern::Mismatch;
}

bool ErrorIndicatorCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	UNREFERENCED_PARAMETER(attr);
	return false;
}

launcherapp::core::Command* ErrorIndicatorCommand::Clone()
{
	return nullptr;
}

bool ErrorIndicatorCommand::Save(CommandEntryIF* entry)
{
	UNREFERENCED_PARAMETER(entry);
	// 実装しない
	return false;
}

bool ErrorIndicatorCommand::Load(CommandEntryIF* entry)
{
	UNREFERENCED_PARAMETER(entry);
	// 実装しない
	return false;
}

uint32_t ErrorIndicatorCommand::AddRef()
{
	return (uint32_t)InterlockedIncrement(&in->mRefCount);
}

uint32_t ErrorIndicatorCommand::Release()
{
	auto n = InterlockedDecrement(&in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
}

}}} // end of namespace launcherapp::commands::error


