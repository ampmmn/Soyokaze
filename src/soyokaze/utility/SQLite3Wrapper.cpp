#include "pch.h"
#include "SQLite3Wrapper.h"
#include "utility/CharConverter.h"
#include <regex>

namespace launcherapp {
namespace utility {

typedef int (__stdcall * LPSQLITE3_OPEN)(const char *, void **);
typedef int (__stdcall * LPSQLITE3_EXEC)(void *, const char *, void*, void *, char **);
typedef int (__stdcall * LPSQLITE3_GET_TABLE)(void *, const char *, char ***, int *, int *, char **);
typedef int (__stdcall * LPSQLITE3_FREE_TABLE)(char **);
typedef int (__stdcall * LPSQLITE3_CLOSE)(void *);
typedef const unsigned char * (__stdcall * LPSQLITE3_VALUE_TEXT)(void*);
typedef void (__stdcall * LPSQLITE3_RESULT_ERROR)(void*, const char*, int);
typedef void (__stdcall * LPSQLITE3_RESULT_INT)(void*, int);
typedef int (__stdcall * LPSQLITE3_CREATE_FUNCTION)(void* ctx, const char*, int, int, void*, void*, void*, void*);
typedef void* (__stdcall * LPSQLITE3_USER_DATA)(void* ctx);

static LPSQLITE3_OPEN sqlite3_open = nullptr;
static LPSQLITE3_EXEC sqlite3_exec = nullptr;
static LPSQLITE3_GET_TABLE sqlite3_get_table = nullptr;
static LPSQLITE3_FREE_TABLE sqlite3_free_table = nullptr;
static LPSQLITE3_CLOSE sqlite3_close = nullptr;
static LPSQLITE3_VALUE_TEXT sqlite3_value_text = nullptr;
static LPSQLITE3_RESULT_ERROR sqlite3_result_error = nullptr;
static LPSQLITE3_RESULT_INT sqlite3_result_int = nullptr;
static LPSQLITE3_CREATE_FUNCTION sqlite3_create_function = nullptr;
static LPSQLITE3_USER_DATA sqlite3_user_data = nullptr;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct SQLite3Wrapper::PImpl
{
	HMODULE mModule = nullptr;
	CharConverter mConv;
	tregex mRegExp;
	bool mIsFirst = true;
};


SQLite3Wrapper::SQLite3Wrapper() : in(new PImpl)
{
	HMODULE lib = LoadLibrary(_T("winsqlite3.dll"));
	in->mModule = lib;

	if (lib) {
		sqlite3_open = (LPSQLITE3_OPEN)GetProcAddress(lib, "sqlite3_open");
		sqlite3_exec = (LPSQLITE3_EXEC)GetProcAddress(lib, "sqlite3_exec");
		sqlite3_get_table = (LPSQLITE3_GET_TABLE)GetProcAddress(lib, "sqlite3_get_table");
		sqlite3_free_table = (LPSQLITE3_FREE_TABLE)GetProcAddress(lib, "sqlite3_free_table");
		sqlite3_close = (LPSQLITE3_CLOSE)GetProcAddress(lib, "sqlite3_close");
		sqlite3_value_text = (LPSQLITE3_VALUE_TEXT )GetProcAddress(lib, "sqlite3_value_text");
		sqlite3_result_error = (LPSQLITE3_RESULT_ERROR )GetProcAddress(lib, "sqlite3_result_error");
		sqlite3_result_int = (LPSQLITE3_RESULT_INT )GetProcAddress(lib, "sqlite3_result_int");
		sqlite3_create_function = (LPSQLITE3_CREATE_FUNCTION)GetProcAddress(lib, "sqlite3_create_function");
		sqlite3_user_data =  (LPSQLITE3_USER_DATA)GetProcAddress(lib, "sqlite3_user_data");
	}
	else {
		spdlog::error(_T("Failed to load winsqlite3.dll!"));
	}

}

SQLite3Wrapper::~SQLite3Wrapper()
{
	if (in->mModule) {
		FreeLibrary(in->mModule);
	}
}

void SQLite3Wrapper::CallbackRegexp(void* ctx, int argc, void** values)
{
	auto pThis = (SQLite3Wrapper*)sqlite3_user_data(ctx);
	pThis->MatchRegExp(ctx, argc, values);
}

void SQLite3Wrapper::MatchRegExp(void* ctx, int argc, void** values)
{
	char* reg = (char*)sqlite3_value_text(values[0]);
	char* text = (char*)sqlite3_value_text(values[1]);

	if ( argc != 2 || reg == 0 || text == 0) {
		sqlite3_result_error(ctx, "SQL function regexp() called with invalid arguments.\n", -1);
		return;
	}


	CString str;
	if (in->mIsFirst) {
		in->mConv.Convert(reg, str);
		in->mRegExp = tregex(str);
		in->mIsFirst = false;
	}

	in->mConv.Convert(text, str);
	bool isMatch = std::regex_search(tstring(str), in->mRegExp);
	sqlite3_result_int(ctx, isMatch ? 1 : 0);
}

int SQLite3Wrapper::Open(const CString& filePath, void** ctx)
{
	CStringA filePathA;
	CharConverter conv(CP_ACP);
	conv.Convert(filePath, filePathA);

	int n = sqlite3_open(filePathA, ctx);
	if (*ctx != nullptr) {
		sqlite3_create_function(*ctx, "regexp", 2, 5, this, &CallbackRegexp,0,0);
	}
	return n;
}

int SQLite3Wrapper::Exec(void *ctx, const CString& queryStr, void* callback, void * param, char **err)
{
	in->mRegExp = tregex();
	in->mIsFirst = true;

	CStringA queryStrA;

	CharConverter conv;
	conv.Convert(queryStr, queryStrA);
	return sqlite3_exec(ctx, queryStrA, callback, param, err);
}

int SQLite3Wrapper::Close(void *ctx)
{
	return sqlite3_close(ctx);
}

SQLite3Wrapper* SQLite3Wrapper::Get()
{
	static SQLite3Wrapper inst;
	return &inst;
}

} // end of namespace utility
} // end of namespace launcherapp

