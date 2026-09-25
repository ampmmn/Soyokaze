#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/SHA1.h"

TEST(SHA1, test1)
{
	SHA1 sha;
	CString out = sha.Finish();
	EXPECT_STREQ(_T("da39a3ee"), (LPCTSTR)out);
}

TEST(SHA1, test2)
{
	SHA1 sha;
	LPCSTR p = "The quick brown fox jumps over the lazy dog";
	std::vector<uint8_t> in;
	in.insert(in.end(), (uint8_t*)p, (uint8_t*)(p + strlen(p)));
	sha.Add(in);
	CString out = sha.Finish();
	EXPECT_STREQ(_T("2fd4e1c6"), (LPCTSTR)out);
}

TEST(SHA1, Finish_FullDigest)
{
	SHA1 sha;
	LPCSTR p = "The quick brown fox jumps over the lazy dog";
	std::vector<uint8_t> in;
	in.insert(in.end(), (uint8_t*)p, (uint8_t*)(p + strlen(p)));
	sha.Add(in);
	CString out = sha.Finish(true);
	EXPECT_STREQ(_T("2fd4e1c67a2d28fced849ee1bb76e7391b93eb12"), (LPCTSTR)out);
}
