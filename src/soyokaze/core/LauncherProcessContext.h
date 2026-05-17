#pragma once

namespace launcherapp { namespace core {

class LauncherProcessContext
{
	LauncherProcessContext();
	~LauncherProcessContext();

public:
	static LauncherProcessContext* GetInstance(); 

	// 先行プロセスである旨をセット
	void MarkAsPrimaryProcess();
	// シャットダウン中である旨をセット
	void MarkShutdownInProgress();

	// 先行プロセスか?
	bool IsPrimaryProcess();
	// シャットダウン中か?
	bool IsShutdownInProgress();
};


}} // end of namespace launcherapp::core

