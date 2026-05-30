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

void DDX_CBIndex(CDataExchange* pDX, int nIDC, bool& is)
{
	int n = is ? 1 : 0;
	DDX_CBIndex(pDX, nIDC, n);
	ASSERT(n == 0 || n == 1 || n == -1);
	is = (n != 0);
}
