#include "pch.h"
#include "UserCommandBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace common {

UserCommandBase::UserCommandBase()
{
}

UserCommandBase::~UserCommandBase()
{
}

bool UserCommandBase::IsEditable()
{
	return true;
}

bool UserCommandBase::IsDeletable()
{
	return true;
}

uint32_t UserCommandBase::AddRef()
{
	return ++mRefCount;
}

uint32_t UserCommandBase::Release()
{
	uint32_t n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

}
}
}

