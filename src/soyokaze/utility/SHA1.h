#pragma once

#include <memory>

class SHA1
{
public:
	SHA1();
	~SHA1();

	void Add(const std::vector<uint8_t>& data);
	void Add(const CString& data);
	/**
	  SHA-1ダイジェストを16進数文字列として取得する
	  @return fullDigestがfalseの場合は先頭4バイト、trueの場合は全20バイトの値
	  @param[in] fullDigest trueの場合は切り詰めずに取得する
	*/
	CString Finish(bool fullDigest = false);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

