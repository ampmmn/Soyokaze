#pragma once

#include <vector>


class Pipe
{
public:
	using Buffer = std::vector<char>;
public:
	Pipe();
	~Pipe();

	HANDLE GetWriteHandle();
	HANDLE GetReadHandle();
	size_t ReadAll(Buffer& buff);

protected:
	HANDLE mWriteHandle;
	HANDLE mReadHandle;
};


