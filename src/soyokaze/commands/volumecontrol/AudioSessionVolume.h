#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace volumecontrol {


class AudioSessionVolume
{
public:
	AudioSessionVolume();
	~AudioSessionVolume();

	HRESULT GetVolume(int* level);
	HRESULT SetVolume(int level);
	HRESULT GetMute(BOOL* isMute);
	HRESULT SetMute(BOOL isMute);

protected:
	HRESULT Initialize();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

