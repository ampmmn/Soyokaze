#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/AES.h"

using AES = launcherapp::utility::aes::AES;

TEST(AES, testInit)
{
	AES aes;

}

TEST(AES, testSetPassphrase)
{
	AES aes;

	bool isOK = aes.SetPassphrase("aiueo");
	EXPECT_TRUE(isOK);
}

TEST(AES, testEncrypt)
{
	AES aes;
	aes.SetPassphrase("aiueo");

	std::vector<BYTE> plain{ 'a', 'b', 'c' };
	std::vector<BYTE> encrypted;
	bool isOK = aes.Encrypt(plain, encrypted);

	EXPECT_TRUE(isOK);
	EXPECT_FALSE(encrypted.empty());
	EXPECT_TRUE(plain != encrypted);
}

TEST(AES, testDescrypt)
{
	AES aes;
	aes.SetPassphrase("aiueo");

	std::vector<BYTE> plain{ 'a', 'b', 'c' };
	std::vector<BYTE> encrypted;
	aes.Encrypt(plain, encrypted);

	std::vector<BYTE> replain;
	bool isOK = aes.Decrypt(encrypted, replain);

	EXPECT_TRUE(isOK);
	EXPECT_TRUE(plain == replain);
}
