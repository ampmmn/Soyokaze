#pragma once

#include "core/CommandIF.h"
#include "core/CommandQueryItem.h"
#include <map>
#include <vector>

class CommandMap
{
public:
	using CommandQueryItem = soyokaze::CommandQueryItem;
	using CommandQueryItemList = soyokaze::CommandQueryItemList;

public:
	CommandMap();
	CommandMap(const CommandMap& rhs);
	~CommandMap();

	bool Serialize(CommandFile* file);

	void Clear();

	bool Has(const CString& name) const;
	soyokaze::core::Command* Get(const CString& name);

	// 登録/解除
	void Register(soyokaze::core::Command* cmd);
	bool Unregister(soyokaze::core::Command* cmd);
	bool Unregister(const CString& name);
	// リネーム
	bool Reregister(soyokaze::core::Command* cmd);

	void Swap(CommandMap& rhs);

	void Query(Pattern* pattern, CommandQueryItemList& commands);

	// 最初に見つけた要素を返す
	soyokaze::core::Command* FindOne(Pattern* pattern);

	// 配列化する
	std::vector<soyokaze::core::Command*>& Enumerate(std::vector<soyokaze::core::Command*>& commands);

protected:
	std::map<CString, soyokaze::core::Command*> mMap;
};
