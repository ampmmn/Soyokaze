#pragma once

#include <stdint.h>

struct COPYDATA_SHELLEXEC
{
	static constexpr int ID = 1;

	uint32_t mVersion = 0;
	int32_t mShowType = 0;
	int32_t mPathOffset = 0;
	int32_t mParamOffset = -1;
	int32_t mWorkDirOffset = -1;
	int32_t mIndexPID = 0;
	TCHAR mData[1] = {};
};

