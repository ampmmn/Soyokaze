#pragma once

#include "CommandIF.h"
#include <map>
#include <vector>

class CommandMap
{
public:
	class QueryItem
	{
	public:
		QueryItem(int level, Command* cmd);
		QueryItem(const QueryItem&) = default;

		int mMatchLevel;
		Command* mCommand;
	};


public:
	CommandMap();
	CommandMap(const CommandMap& rhs);
	~CommandMap();

	void Clear();

	bool Has(const CString& name) const;
	Command* Get(const CString& name);

	// 登録/解除
	void Register(Command* cmd);
	bool Unregister(Command* cmd);
	bool Unregister(const CString& name);

	void Swap(CommandMap& rhs);

	void Query(Pattern* pattern, std::vector<QueryItem>& commands);

	// 最初に見つけた要素を返す
	Command* FindOne(Pattern* pattern);

	// 配列化する
	std::vector<Command*>& Enumerate(std::vector<Command*>& commands);

protected:
	std::map<CString, Command*> mMap;
};
