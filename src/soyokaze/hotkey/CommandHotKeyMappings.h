#pragma once

#include <memory>

class CommandHotKeyAttribute;

class CommandHotKeyMappings
{
public:
	CommandHotKeyMappings();
	~CommandHotKeyMappings();

public:
	int GetItemCount() const;
	CString GetName(int index) const;
	void GetHotKeyAttr(int index, CommandHotKeyAttribute& hotKeyAttr) const;
	void AddItem(const CString& name, const CommandHotKeyAttribute& hotKeyAttr);
	bool RemoveItem(const CString& name);

	// コマンド名から割り当てキーの表示用文字列を取得する
	CString FindKeyMappingString(const CString& name) const;

	void Swap(CommandHotKeyMappings& rhs);


private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

