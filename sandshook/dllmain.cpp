﻿// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "pch.h"
#include <tchar.h>
#include <stdint.h>
#include <stdio.h>

struct SandSKeyState
{
	bool ResetIfNeeded() {
		auto curTickCount = GetTickCount64();
		if (curTickCount - mLastCalled <= 2000) {
			// 不要
			return false;
		}

		// 前回呼び出し時から所定の時間が経過していたら、状態を戻す
		memset(mKeyPressed, 0, sizeof(mKeyPressed));
		mLastCalled = curTickCount;
		return true;
	}
	bool mKeyPressed[256] = {};
	ULONGLONG mLastCalled = GetTickCount64();
};

static SandSKeyState keyState;

// DLLのインスタンスハンドル
static HINSTANCE hDll = nullptr;
// フック登録時のハンドル
static HHOOK hHook = nullptr;
//
static HANDLE hMapFile = nullptr;

BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD ul_reason_for_call,
	LPVOID 
)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		hDll = hModule;
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static uint8_t ScanToVK(UINT scanCode)
{
	return (uint8_t)MapVirtualKeyA(scanCode, MAPVK_VSC_TO_VK_EX);
}


static LRESULT CALLBACK OnKeyHookProc(
	int code,
	WPARAM wp,
	LPARAM lp
)
{
	// 前回のキー状態変化時から所定の時間が経過している場合は内部状態をいったんリセット
	// (他ツールと干渉している可能性もあるが、内部状態的にキーが押下されたまま戻らないことがあるケースへの対策)
	keyState.ResetIfNeeded();

	KBDLLHOOKSTRUCT* info = (KBDLLHOOKSTRUCT*)lp;
	if ((info->flags & LLKHF_INJECTED) != 0) {
		return CallNextHookEx(hHook, code, wp, lp);
	}

	int vkCode = ScanToVK(info->scanCode);
	bool isPressed = (info->flags & LLKHF_UP) == 0;
	keyState.mKeyPressed[(uint8_t)vkCode] = isPressed;

	return CallNextHookEx(hHook, code, wp, lp);
}

/**
 	OSにキーフック関数を登録する 
	@return 正常終了時0 エラー時1
*/
extern "C"
__declspec(dllexport)
LRESULT sands_RegisterHook()
{
	hHook = SetWindowsHookEx(WH_KEYBOARD_LL, OnKeyHookProc, hDll, 0);
	if (hHook == nullptr) {
		return 1;
	}

	return true;
}

/**
 	登録したフック関数を解除
	@return 正常終了時0 エラー時1
*/
extern "C"
__declspec(dllexport)
LRESULT sands_UnregisterHook()
{
	BOOL b = UnhookWindowsHookEx(hHook);
	hHook = nullptr;
	return b? 0: 1;
}

extern "C"
__declspec(dllexport) 
int
sands_IsPressed(UINT modKeyCode, UINT keyCode)
{
	if (keyState.mKeyPressed[(uint8_t)modKeyCode] == false || 
	    keyState.mKeyPressed[(uint8_t)keyCode] == false) {
		return false;
	}
	return true;
}

