#pragma once

#include "commands/core/CommandEntryIF.h"

class CommandFileEntry : public CommandEntryIF
{
public:
	CommandFileEntry();
	~CommandFileEntry();

public:
	void Save(CStdioFile& file);

	void SetName(LPCTSTR name);

	CString GetName() override;

	void MarkAsUsed() override;
	bool IsUsedEntry() override;

	bool HasValue(LPCTSTR key) override;

	int GetValueType(LPCTSTR key) override;

	int Get(LPCTSTR key, int defValue) override;
	void Set(LPCTSTR key, int value) override;

	double Get(LPCTSTR key, double defValue) override;
	void Set(LPCTSTR key, double value) override;

	CString Get(LPCTSTR key, LPCTSTR defValue) override;
	void Set(LPCTSTR key, const CString& value) override;

	bool Get(LPCTSTR key, bool defValue) override;
	void Set(LPCTSTR key, bool value) override;

	bool Get(LPCTSTR key, std::vector<uint8_t>& value) override;
	void Set(LPCTSTR key, const std::vector<uint8_t>& value) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

