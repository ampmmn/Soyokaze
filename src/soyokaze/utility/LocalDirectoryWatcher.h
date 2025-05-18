#pragma once

#include <stdint.h>
#include <memory>

// ローカルファイルの監視
class LocalDirectoryWatcher
{
public:
	typedef void (*LPCALLBACKFUNC)(void* param);
	struct Element;
private:
	LocalDirectoryWatcher();
	~LocalDirectoryWatcher();
public:
	static LocalDirectoryWatcher* GetInstance();

	void Finalize();

	// 登録
	uint32_t Register(LPCTSTR pathToDir, LPCALLBACKFUNC func, void* param);
	// 登録を解除
	bool Unregister(uint32_t id);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

