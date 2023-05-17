#pragma once

class SharedHwnd
{
public:
	SharedHwnd(HWND hwnd) : m_hMapFile(NULL), m_phwnd(NULL)
	{
		m_hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(HWND), _T("SoyokazeWindowHandle"));
		m_phwnd = (HWND*) MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HWND));
		SetHwnd(hwnd);
	}

	SharedHwnd() : m_hMapFile(NULL), m_phwnd(NULL)
	{
		m_hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("SoyokazeWindowHandle"));
		m_phwnd = (HWND*) MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HWND));
	}

	~SharedHwnd()
	{
		UnmapViewOfFile(m_phwnd);
		if (m_hMapFile) {
			CloseHandle(m_hMapFile);
		}
	}

void SetHwnd(HWND hwnd)
{
	if (m_phwnd) {
		*m_phwnd = hwnd;
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

