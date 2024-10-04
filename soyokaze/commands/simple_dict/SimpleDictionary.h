#pragma once

#include <vector>

namespace launcherapp {
namespace commands {
namespace simple_dict {

class Record
{
public:
	Record(const CString& key, const CString& value) : mKey(key), mValue(value) {}
	Record(const Record&) = default;

	CString mKey;
	CString mValue;
};

class Dictionary
{
public:
	std::vector<Record> mRecords;
};


}
}
}

