#pragma once

#include "core/UnknownIF.h"

namespace launcherapp { namespace actions { namespace core {

class Parameter : virtual public launcherapp::core::UnknownIF
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
	virtual Parameter* Clone() const = 0;
};

class NamedParameter : virtual public launcherapp::core::UnknownIF
{
public:
	virtual int GetNamedParamStringLength(LPCTSTR name) const = 0;
	virtual LPCTSTR GetNamedParamString(LPCTSTR name, LPTSTR buf, int bufLen) const = 0;
	virtual void SetNamedParamString(LPCTSTR name, LPCTSTR value) = 0;
	virtual bool GetNamedParamBool(LPCTSTR name) const = 0;
	virtual void SetNamedParamBool(LPCTSTR name, bool value) = 0;
};

}}}
