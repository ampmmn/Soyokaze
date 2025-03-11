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
	int Prepare(void* ctx, const char* sql, void** stmt);
	int Close(void *ctx);

	int BindText(void* stmt, int index, const CStringA& text);
	int Step(void* stmt);
	int Reset(void* stmt);
	int Finalize(void* stmt);

	bool TableExists(void* ctx, const CString& tableName);

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

