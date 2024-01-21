#pragma once

namespace soyokaze {
namespace commands {
namespace watchpath {

class PathWatcher
{
private:
	PathWatcher();
	~PathWatcher();

public:
	static PathWatcher* Get();

	void RegisterPath(const CString& cmdName, const CString& path);
	void UnregisterPath(const CString& cmdName);
	
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

