#include "pch.h"
#include "AppSound.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "utility/Sound.h"

using Sound = utility::Sound;

namespace launcherapp {


struct AppSound::PImpl : public AppPreferenceListenerIF
{
	void OnAppFirstBoot() override
	{
	}
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mInputSound = pref->GetInputSoundFile();
		mSelectSound = pref->GetSelectSoundFile();
		mExecuteSound = pref->GetExecuteSoundFile();
	}
	void OnAppExit() override
	{
	}

	CString mInputSound;
	CString mSelectSound;
	CString mExecuteSound;
};



AppSound::AppSound() : in(std::make_unique<PImpl>())
{
		auto pref = AppPreference::Get();
		pref->RegisterListener(in.get());

		in->mInputSound = pref->GetInputSoundFile();
		in->mSelectSound = pref->GetSelectSoundFile();
		in->mExecuteSound = pref->GetExecuteSoundFile();
}

AppSound::~AppSound()
{
		auto pref = AppPreference::Get();
		pref->UnregisterListener(in.get());
}


AppSound* AppSound::Get()
{
	static AppSound inst;
	return &inst;
}


void AppSound::PlayInputSound()
{
	Sound::Get()->PlayAsync(in->mInputSound);
}

void AppSound::PlaySelectSound()
{
	Sound::Get()->PlayAsync(in->mSelectSound);
}

void AppSound::PlayExecuteSound()
{
	Sound::Get()->PlayAsync(in->mExecuteSound);
}


} // end of namespace launcherapp
