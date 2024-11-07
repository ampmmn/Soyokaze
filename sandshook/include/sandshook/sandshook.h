#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllimport) LRESULT sands_RegisterHook(void);
__declspec(dllimport) LRESULT sands_UnregisterHook(void);
__declspec(dllimport) int sands_IsPressed(UINT modKeyCode, UINT keyCode);

typedef LRESULT (*SANDS_REGISTERHOOK)(void);
typedef LRESULT (*SANDS_UNREGISTERHOOK)(void);
typedef LRESULT (*SANDS_ISPRESSED)(UINT modKeyCode, UINT keyCode);

#ifdef __cplusplus
}
#endif
