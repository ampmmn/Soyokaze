#pragma once

#include <vector>
#include <stdint.h>

class CommandEntryIF
{
public:
	virtual ~CommandEntryIF() {}

	virtual CString GetName() = 0;

	virtual void MarkAsUsed() = 0;
	virtual bool IsUsedEntry() = 0;

	virtual bool HasValue(LPCTSTR key) = 0;

	virtual int GetValueType(LPCTSTR key) = 0;

	virtual int Get(LPCTSTR key, int defValue) = 0;
	virtual void Set(LPCTSTR key, int value) = 0;

	virtual double Get(LPCTSTR key, double defValue) = 0;
	virtual void Set(LPCTSTR key, double value) = 0;

	virtual CString Get(LPCTSTR key, LPCTSTR defValue) = 0;
	virtual void Set(LPCTSTR key, const CString& value) = 0;

	virtual bool Get(LPCTSTR key, bool defValue) = 0;
	virtual void Set(LPCTSTR key, bool value) = 0;

	virtual bool Get(LPCTSTR key, std::vector<uint8_t>& value) = 0;
	virtual void Set(LPCTSTR key, const std::vector<uint8_t>& value) = 0;
};

