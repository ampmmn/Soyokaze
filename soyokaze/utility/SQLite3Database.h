#pragma once

#include <memory>

namespace soyokaze {
namespace utility {

class SQLite3Database
{
public:
	typedef int (*LPQUERYCALLBACK)(void *data, int argc, char **argv, char **colName);

public:
	SQLite3Database(LPCTSTR filePath);
	~SQLite3Database();

public:
	int Query(LPCTSTR queryStr, LPQUERYCALLBACK callback, void* param);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



} // end of namespace utility
} // end of namespace soyokaze

