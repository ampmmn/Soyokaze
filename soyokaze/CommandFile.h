#pragma once

#include "core/CommandIF.h"
#include <vector>
#include <memory>

/**
 * コマンドデータをファイルに読んだり書いたりするためのクラス
 */
class CommandFile
{
public:
	class Entry;

	struct Exception {};

	enum {
		TYPE_INT,
		TYPE_DOUBLE,
		TYPE_STRING,
		TYPE_BOOLEAN,
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

	CString GetName(Entry* entry) const;

	void MarkAsUsed(Entry* entry);
	bool IsUsedEntry(Entry* entry) const;

	bool HasValue(Entry* entry, LPCTSTR key) const;

	int GetValueType(Entry* entry, LPCTSTR key) const;

	int Get(Entry* entry, LPCTSTR key, int defValue) const;
	void Set(Entry* entry, LPCTSTR key, int value);

	double Get(Entry* entry, LPCTSTR key, double defValue) const;
	void Set(Entry* entry, LPCTSTR key, double value);

	CString Get(Entry* entry, LPCTSTR key, LPCTSTR defValue) const;
	void Set(Entry* entry, LPCTSTR key, const CString& value);

	bool Get(Entry* entry, LPCTSTR key, bool defValue) const;
	void Set(Entry* entry, LPCTSTR key, bool value);

	void ClearEntries();
	bool Load();
	bool Save();

	// 廃止予定
	bool Load(std::vector<soyokaze::core::Command*>& commands);
	bool Save(const std::vector<soyokaze::core::Command*>& commands);

	static void TrimComment(CString& s);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

