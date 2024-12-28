#include "pch.h"
#include "framework.h"
#include "WindowPosition.h"
#include "utility/AppProfile.h"
#include "utility/Path.h"
#include "app/AppName.h"
#include <utility> // for std::pair

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr int DEFAULT_HEIGHT = 480;

WindowPosition::WindowPosition() : mPosition({}), mIsLoaded(false)
{
	DWORD bufLen = MAX_COMPUTERNAME_LENGTH + 1;
	BOOL ret = GetComputerName(mName.GetBuffer(bufLen), &bufLen);
	mName.ReleaseBuffer();

	if (ret == FALSE) {
		mName = _T("Window");
	}
}

WindowPosition::WindowPosition(LPCTSTR name) : mPosition({}), mIsLoaded(false)
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
	UNREFERENCED_PARAMETER(hm);
	UNREFERENCED_PARAMETER(hdc);

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
 *  ウインドウ位置設定ファイル(.position)の情報からウインドウ位置を復元する
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

	Path path;
	WindowPosition::GetFilePath(mName, path);

	if (path.FileExists() == FALSE) {
		// 設定ファイルが存在しない場合は気休めにデフォルトの設定ファイルを流用する
		WindowPosition::GetFilePath(APPNAME, path);
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
	mPosition = wp;
	mIsLoaded = true;

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
	mIsLoaded = true;
	return true;
}

bool WindowPosition::UpdateExceptHeight(HWND hwnd)
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	if (GetWindowPlacement(hwnd, &wp) == false) {
		return false;
	}

	int orgHeight = mPosition.rcNormalPosition.bottom - mPosition.rcNormalPosition.top; 

	wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + orgHeight;

	mPosition = wp;
	return true;
}

bool WindowPosition::Save()
{
	try {
		Path path;
		WindowPosition::GetFilePath(mName, path);

		UINT nFlags = CFile::modeCreate | CFile::modeWrite | CFile::typeBinary;
		CFile file(path, nFlags);
		file.Write(&mPosition, sizeof(mPosition));
		return true;
	}
	catch(...) {
		return false;
	}
}

void WindowPosition::GetFilePath(LPCTSTR baseName, Path& path)
{
	CAppProfile::GetDirPath(path, path.size());
	path.Append(baseName);
	path.AddExtension(_T(".position"));
}

WINDOWPLACEMENT WindowPosition::GetPosition() const
{
	return mPosition;
}

bool WindowPosition::SetPositionTemporary(HWND hwnd, const CRect& rc)
{
	auto wp = mPosition;
	wp.rcNormalPosition = rc;

	if (IsZoomed(hwnd) == FALSE && IsIconic(hwnd) == FALSE) {
		SetWindowPos(hwnd, nullptr,rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
	}
	return true;

	//return SetWindowPlacement(hwnd, &wp) != FALSE;
}

bool WindowPosition::SyncPosition(HWND hwnd)
{
	if (mIsLoaded == false) {
		GetWindowRect(hwnd, &mPosition.rcNormalPosition);
		mPosition.rcNormalPosition.bottom = mPosition.rcNormalPosition.top + DEFAULT_HEIGHT;

	}
	return SetWindowPlacement(hwnd, &mPosition) != FALSE;  // ToDo これの先にClearContent
}

