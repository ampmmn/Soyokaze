#pragma once

#include <memory>
#include <vector>


namespace launcherapp {
namespace utility {

class LocalPathResolver
{
public:
	LocalPathResolver();
	LocalPathResolver(const LocalPathResolver& rhs);
	~LocalPathResolver();

	LocalPathResolver& operator = (const LocalPathResolver& rhs);

	void ResetPath();
	bool AddPath(LPCTSTR path);

	bool Resolve(CString& path);
	bool Resolve(const CString& path, CString& resolvedPath);

	static void GetSystemPath(std::vector<CString>& paths);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};


}
}

