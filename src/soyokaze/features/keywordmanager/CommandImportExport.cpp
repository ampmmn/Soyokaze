#include "pch.h"
#include "features/keywordmanager/CommandImportExport.h"
#include "commands/core/CommandFile.h"
#include "commands/core/CommandFileEntry.h"
#include "commands/core/CommandProviderRepository.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/UserCommandProvider.h"
#include "core/IFIDDefine.h"
#include "features/keywordmanager/CommandImportNameResolver.h"
#include "hotkey/CommandHotKeyManager.h"
#include "setting/AppPreference.h"
#include "utility/RefPtr.h"
#include <algorithm>

using namespace launcherapp::core;

struct CommandImportExport::PImpl
{
	struct ImportCandidate
	{
		RefPtr<Command> mCommand;
		RefPtr<UserCommandProvider> mProvider;
		int mEntryIndex{0};
	};

	// 確認ダイアログ表示中も、名前変更時に参照する元エントリを保持する
	std::unique_ptr<CommandFile> mCommandFile;
	std::vector<ImportCandidate> mCandidates;
	std::vector<Command*> mCandidateCommands;
	std::vector<CString> mSkippedEntryNames;
};

CommandImportExport::CommandImportExport() : in(std::make_unique<PImpl>())
{
}

CommandImportExport::~CommandImportExport()
{
}

bool CommandImportExport::LoadCommands(const CString& filePath)
{
	in->mCandidates.clear();
	in->mCandidateCommands.clear();
	in->mSkippedEntryNames.clear();
	in->mCommandFile = std::make_unique<CommandFile>();
	in->mCommandFile->SetFilePath(filePath);
	if (in->mCommandFile->Load() == false) {
		in->mCommandFile.reset();
		return false;
	}

	auto providerRepository = CommandProviderRepository::GetInstance();
	std::vector<CommandProvider*> providers;
	providerRepository->EnumProviders(providers);

	for (int entryIndex = 0; entryIndex < in->mCommandFile->GetEntryCount(); ++entryIndex) {
		auto entry = in->mCommandFile->GetEntry(entryIndex);
		PImpl::ImportCandidate candidate;
		candidate.mEntryIndex = entryIndex;

		for (auto provider : providers) {
			RefPtr<UserCommandProvider> userProvider;
			if (provider->QueryInterface(IFID_USERCOMMANDPROVIDER, (void**)&userProvider) == false) {
				continue;
			}

			RefPtr<Command> command;
			if (userProvider->LoadFrom(entry, &command) == false || command.get() == nullptr) {
				command.reset();
				continue;
			}

			candidate.mCommand.swap(command);
			candidate.mProvider.swap(userProvider);
			in->mCommandFile->MarkAsUsed(entry);
			break;
		}

		if (candidate.mCommand.get() == nullptr) {
			in->mSkippedEntryNames.push_back(CommandFile::GetName(entry));
			continue;
		}

		in->mCandidateCommands.push_back(candidate.mCommand.get());
		in->mCandidates.push_back(std::move(candidate));
	}

	return true;
}

const std::vector<Command*>& CommandImportExport::GetImportCandidates() const
{
	return in->mCandidateCommands;
}

const std::vector<CString>& CommandImportExport::GetSkippedEntryNames() const
{
	return in->mSkippedEntryNames;
}

std::vector<CString> CommandImportExport::ImportCommands(const std::vector<int>& selectedIndices, bool isOverwriteSelected)
{
	std::vector<CString> importedNames;
	if (in->mCommandFile == nullptr) {
		return importedNames;
	}

	auto cmdRepoPtr = CommandRepository::GetInstance();
	auto hotKeyManager = CommandHotKeyManager::GetInstance();
	for (auto candidateIndex : selectedIndices) {
		if (candidateIndex < 0 || candidateIndex >= (int)in->mCandidates.size()) {
			continue;
		}

		auto& candidate = in->mCandidates[candidateIndex];
		RefPtr<Command> command(candidate.mCommand);
		CString commandName = command->GetName();
		RefPtr<Command> existingCommand(cmdRepoPtr->QueryAsWholeMatch(commandName));

		if (existingCommand.get() != nullptr && isOverwriteSelected) {
			CString existingName = existingCommand->GetName();
			CommandHotKeyMappings previousMappings;
			hotKeyManager->GetMappings(previousMappings);

			CommandHotKeyAttribute previousHotKey;
			bool hasPreviousHotKey = false;
			for (int index = 0; index < previousMappings.GetItemCount(); ++index) {
				if (previousMappings.GetName(index) != existingName) {
					continue;
				}
				previousMappings.GetHotKeyAttr(index, previousHotKey);
				hasPreviousHotKey = true;
				break;
			}

			cmdRepoPtr->UnregisterCommand(existingCommand.get());
			command->AddRef();
			cmdRepoPtr->RegisterCommand(command.get());

			CommandHotKeyMappings currentMappings;
			hotKeyManager->GetMappings(currentMappings);
			bool hasChanged = currentMappings.RemoveItem(commandName);
			if (hasPreviousHotKey) {
				currentMappings.AddItem(commandName, previousHotKey);
				hasChanged = true;
			}
			if (hasChanged) {
				auto preference = AppPreference::Get();
				preference->SetCommandKeyMappings(currentMappings);
				preference->Save();
			}
		}
		else {
			if (existingCommand.get() != nullptr) {
				CString uniqueName = CommandImportNameResolver::GetUniqueName(commandName, [&](const CString& name) {
					RefPtr<Command> existing(cmdRepoPtr->QueryAsWholeMatch(name));
					return existing.get() != nullptr;
				});

				auto entry = static_cast<CommandFileEntry*>(in->mCommandFile->GetEntry(candidate.mEntryIndex));
				entry->SetName(uniqueName);
				RefPtr<Command> renamedCommand;
				if (candidate.mProvider->LoadFrom(entry, &renamedCommand) == false || renamedCommand.get() == nullptr) {
					spdlog::error(_T("Failed to reload command with an import name. name:{}"), (LPCTSTR)uniqueName);
					continue;
				}
				command.swap(renamedCommand);
				commandName = command->GetName();
			}

			command->AddRef();
			cmdRepoPtr->RegisterCommand(command.get());
		}

		if (std::none_of(importedNames.begin(), importedNames.end(), [&](const CString& name) {
			return name.CompareNoCase(commandName) == 0;
		})) {
			importedNames.push_back(commandName);
		}
	}

	return importedNames;
}

CommandImportExport::ExportResult CommandImportExport::ExportCommands(
	const std::vector<Command*>& commands, const CString& filePath)
{
	CommandFile commandFile;
	commandFile.SetFilePath(filePath);
	for (auto command : commands) {
		auto entry = commandFile.NewEntry(command->GetName());
		if (command->Save(entry) == false) {
			return {ExportError::CommandSaveFailed, command->GetName()};
		}
	}

	if (commandFile.Save() == false) {
		return {ExportError::FileSaveFailed, CString()};
	}

	return {};
}
