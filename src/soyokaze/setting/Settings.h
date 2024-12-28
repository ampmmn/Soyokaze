#pragma once

#include <memory>
#include <set>

class Settings
{
public:
	struct Exception {};

	enum {
		TYPE_INT,
		TYPE_DOUBLE,
		TYPE_STRING,
		TYPE_BOOLEAN,
		TYPE_UNKNOWN,
	};

public:
	Settings();
	~Settings();

	int GetType(LPCTSTR key) const;
	bool Has(LPCTSTR key) const;

	void EnumKeys(std::set<CString>& keys) const;

	int Get(LPCTSTR key, int defValue) const;
	void Set(LPCTSTR key, int value);

	double Get(LPCTSTR key, double defValue) const;
	void Set(LPCTSTR key, double value);

	CString Get(LPCTSTR key, LPCTSTR defValue) const;
	void Set(LPCTSTR key, const CString& value);

	bool Get(LPCTSTR key, bool defValue) const;
	void Set(LPCTSTR key, bool value);

	Settings* Clone() const;
	void Swap(Settings& rhs);


private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

