#pragma once

#include <memory>

namespace launcherapp {

// アプリケーションの効果音再生クラス
class AppSound
{
private:
	AppSound();
	~AppSound();

public:
	static AppSound* Get();

	// 入力音の再生
	void PlayInputSound();
	// 選択音の再生
	void PlaySelectSound();
	// 実行音の再生
	void PlayExecuteSound();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace launcherapp

