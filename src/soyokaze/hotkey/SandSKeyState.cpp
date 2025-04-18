#include "pch.h"
#include "SandSKeyState.h"
#include "SharedHwnd.h"
#include "utility/Path.h"
#include "sandshook/sandshook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct SandSKeyState::PImpl
{
	HMODULE mDll{nullptr};
	SANDS_REGISTERHOOK mRegisterHook{nullptr};
	SANDS_UNREGISTERHOOK mUnregisterHook{nullptr};
	SANDS_ISPRESSED mIsPressed{nullptr};
	SANDS_RESET mReset{nullptr};

	bool mLastState[0x10000]{{}};
	bool mIsLogPrinted{false};
};

SandSKeyState::SandSKeyState() : in(new PImpl)
{
	Initialize();
}

SandSKeyState::~SandSKeyState()
{
	Finalize();
}

SandSKeyState* SandSKeyState::GetInstance()
{
	static SandSKeyState inst;
	return &inst;
}


void SandSKeyState::Initialize()
{
	Path filePath(Path::MODULEFILEDIR);
	filePath.Append(_T("sandshook.dll"));

	in->mDll = LoadLibrary((LPCTSTR)filePath);

	in->mRegisterHook = (SANDS_REGISTERHOOK)GetProcAddress(in->mDll, "sands_RegisterHook");
	in->mUnregisterHook = (SANDS_UNREGISTERHOOK)GetProcAddress(in->mDll, "sands_UnregisterHook");
	in->mIsPressed = (SANDS_ISPRESSED)GetProcAddress(in->mDll, "sands_IsPressed");
	in->mReset = (SANDS_RESET)GetProcAddress(in->mDll, "sands_ResetState");

	if (in->mRegisterHook) {
		in->mRegisterHook();
	}
}

void SandSKeyState::Finalize()
{
	if (in->mUnregisterHook) {
		in->mUnregisterHook();
	}
}

bool SandSKeyState::IsPressed(UINT modKeyCode, UINT keyCode)
{
	int index = ((modKeyCode & 0xFF) << 8) | (keyCode & 0xFF);
	ASSERT(index < 0x10000);

	if (in->mIsPressed == nullptr) {
		if (in->mIsLogPrinted == false) {
			spdlog::info("SandS is not available.");
			in->mIsLogPrinted = true;
		}
		return false;
	}

	if (in->mIsPressed(modKeyCode, keyCode) == false) {
		in->mLastState[index] = false;
		return false;
	}

	if (in->mLastState[index]) {
	return false;
	}

	in->mLastState[index] = true;
	return true;
}

void SandSKeyState::Reset()
{
	if (in->mIsPressed == nullptr) {
		if (in->mIsLogPrinted == false) {
			spdlog::info("SandS is not available.");
			in->mIsLogPrinted = true;
		}
		return ;
	}

	in->mReset();
}
