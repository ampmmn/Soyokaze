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
	// アプリの再起動が必要である旨をセット
	void MarkRestartRequired();

	// 先行プロセスか?
	bool IsPrimaryProcess();
	// シャットダウン中か?
	bool IsShutdownInProgress();
	// アプリの再起動が必要か?
	bool IsRestartRequired();
};


}} // end of namespace launcherapp::core

