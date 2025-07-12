#pragma once

#include <memory>
#include <vector>

namespace launcherapp { namespace icon {

class CommandIcon
{
public:
	CommandIcon();
	~CommandIcon();

public:
	bool IsNull();
	operator HICON();
	HICON IconHandle();
	void Reset();

	HICON LoadFromPath(LPCTSTR path);
	HICON LoadFromStream(const std::vector<uint8_t>& stm);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};


}}

