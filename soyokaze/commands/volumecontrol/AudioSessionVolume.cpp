#include "pch.h"
#include "AudioSessionVolume.h"
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>


namespace launcherapp {
namespace commands {
namespace volumecontrol {


static const GUID GUIDEmpty =
{ 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

struct AudioSessionVolume::PImpl
{
	IAudioEndpointVolume* mVol = nullptr;
};

AudioSessionVolume::AudioSessionVolume() : in(new PImpl)
{
	CoInitialize(nullptr);
	Initialize();
}

AudioSessionVolume::~AudioSessionVolume()
{
	if (in->mVol) {
		in->mVol->Release();
	}
	CoUninitialize();
}


HRESULT AudioSessionVolume::Initialize()
{
	IMMDeviceEnumerator *pDeviceEnumerator = nullptr;
	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDeviceEnumerator));
	if (FAILED(hr)) {
		return hr;
	}

	IMMDevice* pDevice = nullptr;
	hr = pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice);
	if (FAILED(hr)) {
		pDeviceEnumerator->Release();
		return hr;
	}

	hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, nullptr, (void**) &in->mVol);
	if (FAILED(hr)) {
		pDevice->Release();
		pDeviceEnumerator->Release();
		return hr;
	}

	pDevice->Release();
	pDeviceEnumerator->Release();

	return hr;
}

HRESULT AudioSessionVolume::GetVolume(int* level)
{
	ASSERT(level);
	if (in->mVol == nullptr) {
		return E_FAIL;
	}

	float f;
	HRESULT hr = in->mVol->GetMasterVolumeLevelScalar(&f);
	if (FAILED(hr)) {
		return hr;
	}

	*level = (int)(f * 100);
	return hr;
}


HRESULT AudioSessionVolume::SetVolume(int level)
{
	if (in->mVol == nullptr) {
		return E_FAIL;
	}

	float f = (float)(level / 100.0);
	if (f < 0.0f) {
		f = 0.0f;
	}
	if (1.0f < f) {
		f = 1.0f;
	}
	return in->mVol->SetMasterVolumeLevelScalar(f, &GUIDEmpty);
}

HRESULT AudioSessionVolume::GetMute(BOOL* isMute)
{
    return in->mVol ? in->mVol->GetMute(isMute) : E_FAIL;
}

HRESULT AudioSessionVolume::SetMute(BOOL isMute) 
{
    return in->mVol ? in->mVol->SetMute(isMute, &GUIDEmpty) : E_FAIL;
}

}
}
}

