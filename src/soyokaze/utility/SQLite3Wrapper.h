#pragma once


namespace launcherapp {
namespace utility {

class SQLite3Wrapper
{
private:
	SQLite3Wrapper();
	~SQLite3Wrapper();

public:
	int Open(const CString& filePath, void** ctx);
	int Exec(void *ctx, const CString& queryStr, void* callback, void * param, char **err);
	int Close(void *ctx);

	static SQLite3Wrapper* Get();

private:
	static void CallbackRegexp(void* ctx, int argc, void** values);
	void MatchRegExp(void* ctx, int argc, void** values);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace utility
} // end of namespace launcherapp

