#include "pch.h"
#include "VersionInfo.h"
#include <vector>


bool VersionInfo::GetVersionInfo(CString& version)
{
	// バージョン情報を取得
	TCHAR szModulePath[MAX_PATH_NTFS];
	GetModuleFileName( NULL, szModulePath, MAX_PATH_NTFS);

	DWORD size = GetFileVersionInfoSize(szModulePath, NULL);
	std::vector<BYTE> versionData(size);

	BYTE* pVersion = &(versionData.front());
	if (GetFileVersionInfo(szModulePath, NULL, size, pVersion) == FALSE) {
		return false;
	}

	UINT actualLen = 0;
	VS_FIXEDFILEINFO* pFileInfo;
	VerQueryValue(pVersion, _T("\\"), (void**)&pFileInfo, &actualLen);

	version.Format(_T("%d.%d.%d"),
			HIWORD(pFileInfo->dwFileVersionMS),
			LOWORD(pFileInfo->dwFileVersionMS),
			HIWORD(pFileInfo->dwFileVersionLS));

	return true;
}


bool VersionInfo::GetBuildDateTime(CTime& tmBuildDate)
{
	TCHAR path[MAX_PATH_NTFS];
	GetModuleFileName(NULL, path, MAX_PATH_NTFS);

	FILE* fp = nullptr;
 	if(_tfopen_s(&fp, path, _T("rb"))!= 0) {
		return false;
	}

	IMAGE_DOS_HEADER hdr;
	if (fread(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
		fclose(fp);
		return false;
	}

	if (hdr.e_lfanew < sizeof(hdr)) {
		fclose(fp);
		return false;
	}

	size_t offset2PE = hdr.e_lfanew - sizeof(hdr);

	uint8_t skipbytes[65536];
	while(offset2PE > 0) {
		size_t skipSize = offset2PE > 65536 ? 65536 : offset2PE;
		if (fread(skipbytes, 1, skipSize, fp) != skipSize) {
			fclose(fp);
			return false;
		}
		offset2PE -= skipSize;
	}

	uint8_t bytes[4];
	if (fread(bytes, 1, 4, fp) != 4) {
		fclose(fp);
		return false;
	}
	if (bytes[0] != 'P' || bytes[1] != 'E' || bytes[2] != '\0'  || bytes[3] != '\0') {
		fclose(fp);
		return false;
	}

	// COFF File Headerからタイムスタンプを読み取る
	uint8_t skipBytes[4];
	if (fread(skipBytes, 1, 4, fp) != 4) {
		fclose(fp);
		return false;
	}

	uint8_t timeStamp[4];
	if (fread(timeStamp, 1, 4, fp) != 4) {
		fclose(fp);
		return false;
	}
	fclose(fp);

	__time64_t offset = *(uint32_t*)timeStamp;

	tmBuildDate = CTime(offset);

	return true;
}

