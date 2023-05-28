#pragma once

#include <memory>

class HOTKEY_ATTR;

class CommandHotKeyMappings
{
public:
	CommandHotKeyMappings();
	~CommandHotKeyMappings();

public:
	int GetItemCount() const;
	CString GetName(int index) const;
	void GetHotKeyAttr(int index, HOTKEY_ATTR& hotKeyAttr) const;
	bool IsGlobal(int index) const;
	void AddItem(const CString& name, const HOTKEY_ATTR& hotKeyAttr, bool isGlobal = false);
	void RemoveItem(const HOTKEY_ATTR& hotKeyAttr);
	void Swap(CommandHotKeyMappings& rhs);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

