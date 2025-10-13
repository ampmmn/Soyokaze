#pragma once

#include <string>

struct KEY_DEFINE 
{
	LPCSTR mKeyName;
	int mVK;
	int mVK2;
};


class VKScanner
{
public:
	VKScanner(const std::string& input);
	~VKScanner();

	bool Get(KEY_DEFINE* key) const;
	bool HasNext() const;
	bool Next(KEY_DEFINE* key);

private:
	bool Get(KEY_DEFINE* key, size_t* nextPos) const;

private:
	std::string mInput;
	size_t mPos{0};
};
