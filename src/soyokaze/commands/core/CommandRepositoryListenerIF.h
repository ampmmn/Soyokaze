#pragma once


namespace launcherapp {
namespace core {

class Command;

class CommandRepositoryListenerIF
{
public:
	virtual ~CommandRepositoryListenerIF() = 0 {}

	virtual void OnBeforeLoad() = 0;
	virtual void OnNewCommand(Command* command) = 0;
	virtual void OnDeleteCommand(Command* command) = 0;
	virtual void OnPatternReloaded() = 0;
};

}
}

