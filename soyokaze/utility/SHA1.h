#pragma once

#include <memory>

class SHA1
{
public:
	SHA1();
	~SHA1();

	void Add(const std::vector<uint8_t>& data);
	CString Finish();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

