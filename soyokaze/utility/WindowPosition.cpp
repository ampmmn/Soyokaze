#include "pch.h"
#include "framework.h"
#include "WindowPosition.h"
#include "utility/AppProfile.h"
#include <utility> // for std::pair

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

WindowPosition::WindowPosition()
{
	DWORD bufLen = MAX_COMPUTERNAME_LENGTH + 1;
	BOOL ret = GetComputerName(mName.GetBuffer(bufLen), &bufLen);
	mName.ReleaseBuffer();

	if (ret == FALSE) {
		mName = _T("Window");
	}
}

WindowPosition::WindowPosition(LPCTSTR name)
{
	ASSERT(name);

	DWORD bufLen = MAX_COMPUTERNAME_LENGTH + 1;
	TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
	if (GetComputerName(computerName, &bufLen)) {
		mName = computerName;
		mName += _T(".");
		mName += name;
	}
	else {
		mName = name;
	}
}

WindowPosition::~WindowPosition()
{
	Save();
}

/**
 * モニタ列挙コールバック関数
 * ウインドウがモニタに収まるかの判定を行う
 *
 * @param hm モニタのハンドル(使わない)
 * @param hdc デバイスコンテキスト(使わない)
 * @param rectMonitor モニタ領域
 * @param lp  ユーザパラメータ(std::pair<RECT,bool>)
 */
static BOOL CALLBACK MonitorCallback(HMONITOR hm, HDC hdc, LPRECT rectMonitor, LPARAM lp)
{
	std::pair<RECT,bool>* param = (std::pair<RECT,bool>*)lp;

	const RECT& rectWnd = param->first;
	bool& isOutOfMonitor = param->second;

	// モニタ領域とウインドウ領域が交差するかどうかで収まっているかを判断する
	RECT rectIntersect;
	if (IntersectRect(&rectIntersect, rectMonitor, &rectWnd)) {
		isOutOfMonitor = false;
	}

	return TRUE;
}

/**
 *  設定ファイル(Soyokaze.position)の情報からウインドウ位置を復元する
 *  前回の位置を復元するために使用する
 *  復元した結果のウインドウ位置がモニター領域に収まっていない場合はfalseを返す
 *
 *  @return true:復元した   false:復元しなかった
 *  @param hwnd 対象ウインドウハンドル
 */
bool WindowPosition::Restore(HWND hwnd)
{
	if (IsWindow(hwnd) == FALSE) {
		return false;
	}

	TCHAR path[MAX_PATH_NTFS];
	GetFilePath(path, MAX_PATH_NTFS);


	if (PathFileExists(path) == FALSE) {
		// 設定ファイルが存在しない場合は気休めにデフォルトの設定ファイルを流用する
		WindowPosition::GetFilePath(_T("Soyokaze"), path, MAX_PATH_NTFS);
	}

	CFile file;
	if (file.Open(path, CFile::modeRead | CFile::shareDenyWrite) == FALSE) {
		return false;
	}

	WINDOWPLACEMENT wp;
	file.Read(&wp, sizeof(wp));

	if (SetWindowPlacement(hwnd, &wp) == FALSE) {
		return FALSE;
	}

	// 各モニタ領域の中に納まっているかをチェック
	RECT rectWnd;
	GetWindowRect(hwnd, &rectWnd);

	std::pair<RECT, bool> param(rectWnd, true);
	EnumDisplayMonitors(NULL, NULL, MonitorCallback, (LPARAM)&param);

	return param.second == false;
}

bool WindowPosition::Update(HWND hwnd)
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	if (GetWindowPlacement(hwnd, &wp) == false) {
		return false;
	}
	mPosition = wp;
	return true;
}

bool WindowPosition::Save()
{
	try {
		TCHAR path[MAX_PATH_NTFS];
		GetFilePath(path, MAX_PATH_NTFS);

		UINT nFlags = CFile::modeCreate | CFile::modeWrite | CFile::typeBinary;
		CFile file(path, nFlags);
		file.Write(&mPosition, sizeof(mPosition));
		return true;
	}
	catch(...) {
		return false;
	}
}


void WindowPosition::GetFilePath(TCHAR* pathBuf, size_t len)
{
	WindowPosition::GetFilePath(mName, pathBuf, len);
}

void WindowPosition::GetFilePath(LPCTSTR baseName, TCHAR* pathBuf, size_t len)
{
	CAppProfile::GetDirPath(pathBuf, len);
	PathAppend(pathBuf, baseName);
	_tcscat_s(pathBuf, len, _T(".position"));
}

