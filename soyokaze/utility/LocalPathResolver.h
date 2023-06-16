#pragma once

#include <memory>


namespace soyokaze {
namespace utility {

class LocalPathResolver
{
public:
	LocalPathResolver();
	LocalPathResolver(const LocalPathResolver& rhs);
	~LocalPathResolver();

	LocalPathResolver& operator = (const LocalPathResolver& rhs);

	bool AddPath(LPCTSTR path);

	bool Resolve(CString& path);
	bool Resolve(const CString& path, CString& resolvedPath);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};


}
}

