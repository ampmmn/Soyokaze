#pragma once

#include <vector>
#include <stdint.h>

class CommandEntryIF
{
public:
	static constexpr auto NO_ENTRY { static_cast<size_t>(-1) };

public:
	virtual ~CommandEntryIF() {}

	virtual LPCTSTR GetName() = 0;

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

	virtual size_t GetBytesLength(LPCTSTR key) = 0;
	virtual bool GetBytes(LPCTSTR key, uint8_t* buf, size_t bufLen) = 0;
	virtual void SetBytes(LPCTSTR key, const uint8_t* buf, size_t bufLen) = 0;
};

