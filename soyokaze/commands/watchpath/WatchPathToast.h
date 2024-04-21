#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace watchpath {

class Toast
{
public:
	Toast();
	~Toast();

	void SetCommandName(const CString& name);
	void SetPath(const CString& path);
	void SetMessage(const CString& message);

	void Show();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

}
}
}

