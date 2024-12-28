#pragma once

#include <vector>

namespace launcherapp {
namespace matcher {

class CommandToken
{
public:
	CommandToken(const CString& commandStr);
	~CommandToken() = default;

	bool GetTrailingString(int endPos, CString& trailingText);

	size_t GetCount() const;

protected:
	void EnumTokenPos(std::vector<int>& tokenPos);

private:
	CString mCommandStr;
	std::vector<int> mTokenPos;
};


}
}

