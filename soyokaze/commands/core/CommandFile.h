#pragma once

#include "commands/core/CommandIF.h"
#include <vector>
#include <memory>

class CommandFileEntry;

/**
 * コマンドデータをファイルに読んだり書いたりするためのクラス
 */
class CommandFile
{
public:
	using Entry = CommandFileEntry;

	struct Exception {};

	enum {
		TYPE_INT,
		TYPE_DOUBLE,
		TYPE_STRING,
		TYPE_BOOLEAN,
		TYPE_STREAM,
		TYPE_UNKNOWN,
	};

public:
	CommandFile();
	~CommandFile();

	void SetFilePath(const CString& filePath);

public:
	int GetEntryCount() const;


	Entry* NewEntry(const CString& name);
	Entry* GetEntry(int index) const;	

	static CString GetName(Entry* entry);

	void MarkAsUsed(Entry* entry);
	static bool IsUsedEntry(Entry* entry);

	static bool HasValue(Entry* entry, LPCTSTR key);

	static int GetValueType(Entry* entry, LPCTSTR key);

	static int Get(Entry* entry, LPCTSTR key, int defValue);
	void Set(Entry* entry, LPCTSTR key, int value);

	static double Get(Entry* entry, LPCTSTR key, double defValue);
	void Set(Entry* entry, LPCTSTR key, double value);

	static CString Get(Entry* entry, LPCTSTR key, LPCTSTR defValue);
	void Set(Entry* entry, LPCTSTR key, const CString& value);

	static bool Get(Entry* entry, LPCTSTR key, bool defValue);
	void Set(Entry* entry, LPCTSTR key, bool value);

	static bool Get(Entry* entry, LPCTSTR key, std::vector<uint8_t>& value);
	void Set(Entry* entry, LPCTSTR key, const std::vector<uint8_t>& value);

	bool Load();
	bool Save();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

