#include "pch.h"
#include "features/keywordmanager/CommandImportExport.h"
#include "commands/core/CommandFile.h"
#include "commands/core/CommandFileEntry.h"
#include "commands/core/CommandProviderRepository.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/UserCommandProvider.h"
#include "core/IFIDDefine.h"
#include "features/keywordmanager/CommandImportNameResolver.h"
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

/**
  選択されたコマンドを一括で登録する
  上書き指定時は既存コマンドを解除対象に含め、登録と解除をまとめて RegisterCommands に渡す
  @param[in] selectedIndices     取り込み対象の候補インデックス
  @param[in] isOverwriteSelected 同名コマンドを上書きするか
  @return 取り込んだコマンド名の一覧
*/
std::vector<CString> CommandImportExport::ImportCommands(const std::vector<int>& selectedIndices, bool isOverwriteSelected)
{
	std::vector<CString> importedNames;
	if (in->mCommandFile == nullptr) {
		return importedNames;
	}

	auto cmdRepoPtr = CommandRepository::GetInstance();
	std::vector<Command*> commandsToRegister;
	std::vector<Command*> commandsToUnregister;
	std::vector<RefPtr<Command>> commandRefs;
	std::vector<RefPtr<Command>> existingCommandRefs;

	// 選択された候補を順に確認し、登録対象と解除対象に振り分ける
	for (auto candidateIndex : selectedIndices) {
		if (candidateIndex < 0 || candidateIndex >= (int)in->mCandidates.size()) {
			continue;
		}

		auto& candidate = in->mCandidates[candidateIndex];
		RefPtr<Command> command(candidate.mCommand);
		CString commandName = command->GetName();

		// 同一インポート内で同名の候補が既にあるかを確認する
		auto stagedCommandIt = std::find_if(commandsToRegister.begin(), commandsToRegister.end(), [&](Command* stagedCommand) {
			return stagedCommand->GetName().CompareNoCase(commandName) == 0;
		});
		bool hasStagedCommand = stagedCommandIt != commandsToRegister.end();
		RefPtr<Command> existingCommand(cmdRepoPtr->QueryAsWholeMatch(commandName));

		// 既存コマンドを上書きする場合は解除対象に加える
		if (existingCommand.get() != nullptr && isOverwriteSelected && hasStagedCommand == false) {
			commandsToUnregister.push_back(existingCommand.get());
			existingCommandRefs.push_back(existingCommand);
		}
		else if (isOverwriteSelected == false) {
			if (existingCommand.get() != nullptr || hasStagedCommand) {
				// 同名を避けるため、一意な名前で読み直す
				CString uniqueName = CommandImportNameResolver::GetUniqueName(commandName, [&](const CString& name) {
					RefPtr<Command> existing(cmdRepoPtr->QueryAsWholeMatch(name));
					if (existing.get() != nullptr) {
						return true;
					}
					return std::any_of(commandsToRegister.begin(), commandsToRegister.end(), [&](Command* stagedCommand) {
						return stagedCommand->GetName().CompareNoCase(name) == 0;
					});
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
		}

		// 同一インポート内で同名の候補が既にあれば、後から読んだもので置き換える
		if (hasStagedCommand && isOverwriteSelected) {
			*stagedCommandIt = command.get();
			commandRefs[stagedCommandIt - commandsToRegister.begin()] = command;
		}
		else {
			commandRefs.push_back(command);
			commandsToRegister.push_back(command.get());
		}

		if (std::none_of(importedNames.begin(), importedNames.end(), [&](const CString& name) {
			return name.CompareNoCase(commandName) == 0;
		})) {
			importedNames.push_back(commandName);
		}
	}

	// 登録する分の参照カウントを増やし、登録と解除をまとめて反映する
	for (auto command : commandsToRegister) {
		command->AddRef();
	}
	cmdRepoPtr->RegisterCommands(commandsToRegister, commandsToUnregister);

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
