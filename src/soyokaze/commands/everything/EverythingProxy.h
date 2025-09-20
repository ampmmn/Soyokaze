#pragma once

#include <memory>
#include <vector>

#include "commands/everything/EverythingResult.h"

namespace launcherapp {
namespace commands {
namespace everything {

// キャンセルされたかどうかのフラグ管理
class CancellationToken
{
public:
	virtual ~CancellationToken() = default;
	// キャンセルが発生したか
	virtual bool IsCancellationRequested() = 0;

};

class EverythingProxy
{
private:
	EverythingProxy();
	~EverythingProxy();

public:
	static EverythingProxy* Get();

	bool Query(const CString& queryStr, CancellationToken* cancelToken, std::vector<EverythingResult>& results);

	HICON GetIcon();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

