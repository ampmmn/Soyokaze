#include "pch.h"
#include "SQLite3Database.h"
#include "utility/SQLite3Wrapper.h"
#include "utility/CharConverter.h"

namespace launcherapp {
namespace utility {

struct SQLite3Statement::PImpl
{
	void* mStmt = nullptr;
	CharConverter mConv;
};

SQLite3Statement::SQLite3Statement() : in(new PImpl)
{
}
SQLite3Statement::~SQLite3Statement()
{
	Finalize();
}

int SQLite3Statement::BindText(int index, const CString& str)
{
	CStringA dst;
	in->mConv.Convert(str, dst);
	return SQLite3Wrapper::Get()->BindText(in->mStmt, index, dst);
}

int SQLite3Statement::Step()
{
	return SQLite3Wrapper::Get()->Step(in->mStmt);
}

int SQLite3Statement::Reset()
{
	return SQLite3Wrapper::Get()->Reset(in->mStmt);
}

int SQLite3Statement::Finalize()
{
	int n = -1;
	if (in->mStmt) {
		n = SQLite3Wrapper::Get()->Finalize(in->mStmt);
		in->mStmt = nullptr;
	}
	return n;
}

void SQLite3Statement::SetStatement(void* stmt)
{
	in->mStmt = stmt;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct SQLite3Database::PImpl
{
	void* mDB = nullptr;
};

SQLite3Database::SQLite3Database(LPCTSTR filePath) : in(new PImpl)
{
	SQLite3Wrapper::Get()->Open(filePath, &in->mDB);
}

SQLite3Database::~SQLite3Database()
{
	if (in->mDB) {
		SQLite3Wrapper::Get()->Close(&in->mDB);
		in->mDB = nullptr;
	} 
}

int SQLite3Database::Query(LPCTSTR queryStr, LPQUERYCALLBACK callback, void* param)
{
	if (in->mDB) {
		char* err = nullptr;
		int n = SQLite3Wrapper::Get()->Exec(in->mDB, queryStr, callback, param, &err);
		return n;
	}
	return -1;
}

int SQLite3Database::Prepare(LPCSTR sql, SQLite3Statement* stmt)
{
	void* stmt_ = nullptr;
	int n = SQLite3Wrapper::Get()->Prepare(in->mDB, sql, &stmt_);
	stmt->SetStatement(stmt_);

	return n;
}

bool SQLite3Database::TableExists(LPCTSTR tableName)
{
	return SQLite3Wrapper::Get()->TableExists(in->mDB, tableName);
}

} // end of namespace utility
} // end of namespace launcherapp

