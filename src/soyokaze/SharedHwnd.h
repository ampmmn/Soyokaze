#pragma once

#include <tchar.h>

class SharedHwnd
{
public:
	SharedHwnd(HWND hwnd) : m_hMapFile(nullptr), m_phwnd(nullptr)
	{
		m_hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(HWND), _T("LauncherAppWindowHandle"));
		if (m_hMapFile != nullptr) {
			m_phwnd = (HWND*) MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HWND));
			SetHwnd(hwnd);
		}
	}

	SharedHwnd() : m_hMapFile(nullptr), m_phwnd(nullptr)
	{
		m_hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("LauncherAppWindowHandle"));
		if (m_hMapFile != nullptr) {
			m_phwnd = (HWND*) MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HWND));
		}
	}

	~SharedHwnd()
	{
		if (m_phwnd) {
			UnmapViewOfFile(m_phwnd);
		}
		if (m_hMapFile) {
			CloseHandle(m_hMapFile);
		}
	}

	void SetHwnd(HWND hwnd)
	{
		if (m_phwnd) {
			*m_phwnd = hwnd;
		}
		else {
			spdlog::error("Failed to set shared hwnd!");
		}
	}

	HWND GetHwnd()
	{
		return m_phwnd ? *m_phwnd : NULL;
	}

protected:
	HANDLE m_hMapFile;
	HWND* m_phwnd;
};

