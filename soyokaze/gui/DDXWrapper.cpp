#include "pch.h"
#include "DDXWrapper.h"

void DDX_Check(CDataExchange* pDX, int nIDC, bool& isChecked)
{
	int n = isChecked;
	DDX_Check(pDX, nIDC, n);
	isChecked = (n != FALSE);
}

void DDX_CBIndex(CDataExchange* pDX, int nIDC, short& index)
{
	int n = index;
	DDX_CBIndex(pDX, nIDC, n);
	ASSERT(n == -1 || n <= 32767);
	index = (short)n;
}
