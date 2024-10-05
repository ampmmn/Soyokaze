#pragma once

#include "commands/core/UnknownIF.h"

namespace launcherapp {
namespace core {

class CommandParameter : virtual public UnknownIF
{
public:
	virtual bool IsEmpty() const = 0;
	virtual bool HasParameter() const = 0;
	virtual LPCTSTR GetWholeString() const = 0;
	virtual LPCTSTR GetCommandString() const = 0;
	virtual LPCTSTR GetParameterString() const = 0;
	virtual int GetParamCount() const = 0;
	virtual LPCTSTR GetParam(int index) const = 0;
	virtual void SetWholeString(LPCTSTR param) = 0;
	virtual void SetParameterString(LPCTSTR param) = 0;
	virtual CommandParameter* Clone() const = 0;
};

class CommandNamedParameter : virtual public UnknownIF
{
public:
	virtual int GetNamedParamStringLength(LPCTSTR name) const = 0;
	virtual LPCTSTR GetNamedParamString(LPCTSTR name, LPTSTR buf, int bufLen) const = 0;
	virtual bool GetNamedParamBool(LPCTSTR name) const = 0;
};

}
}

