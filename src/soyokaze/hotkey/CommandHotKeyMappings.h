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

	// 名前と属性の組み合わせが同一かどうかを比較する(項目の並び順は無視する)
	bool operator == (const CommandHotKeyMappings& rhs) const;
	bool operator != (const CommandHotKeyMappings& rhs) const;


private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

