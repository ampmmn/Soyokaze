#include "pch.h"
#include "SQLite3Database.h"
#include "utility/SQLite3Wrapper.h"

namespace launcherapp {
namespace utility {


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

} // end of namespace utility
} // end of namespace launcherapp

