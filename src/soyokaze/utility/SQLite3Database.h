#pragma once

#include <memory>

namespace launcherapp {
namespace utility {

class SQLite3Statement
{
public:
	SQLite3Statement();
	~SQLite3Statement();

	int BindText(int index, const CString& str);
	int Step();
	int Reset();
	int Finalize();

	void SetStatement(void* stmt);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

class SQLite3Database
{
public:
	typedef int (*LPQUERYCALLBACK)(void *data, int argc, char **argv, char **colName);
	typedef int (__stdcall*LPPROGRESSHANDLER)(void* param);

public:
	SQLite3Database(LPCTSTR filePath, bool isReadOnly = false);
	~SQLite3Database();

public:
	int Query(LPCTSTR queryStr, LPQUERYCALLBACK callback, void* param);
	int Prepare(LPCSTR sql, SQLite3Statement* stmt);
	bool TableExists(LPCTSTR tableName);
	void SetProgressHandler(int n, LPPROGRESSHANDLER handler, void* param);
	void Close();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



} // end of namespace utility
} // end of namespace launcherapp

