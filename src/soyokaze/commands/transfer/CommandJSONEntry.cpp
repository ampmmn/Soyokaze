#include "pch.h"
#include "CommandJSONEntry.h"
#include "utility/CharConverter.h"
#include "utility/Base64.h"
#include <nlohmann/json.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using json = nlohmann::json;

namespace launcherapp {
namespace commands {
namespace transfer {

using json = nlohmann::json;

inline void UTF2UTF(LPCTSTR src, std::string& dst)
{
	UTF2UTF(CString(src), dst);
}

struct CommandJSONEntry::PImpl
{
  /**
   * @brief PImplを初期化する
   */
	PImpl(LPCTSTR name) : mName(name), mIsUsed(false)
	{
		UTF2UTF(mName, mNameInUTF8);
	}
	PImpl() : mIsUsed(false)
	{
	}

	std::string GetNameInUTF8() {
		return mNameInUTF8;
	}

	CString mName;
	std::string mNameInUTF8;
	json mData;
	bool mIsUsed;
	uint32_t mRefCount{1};
};

// Public methods
CommandJSONEntry::CommandJSONEntry(LPCTSTR name) : in(std::make_unique<PImpl>(name))
{
    Init();
}

CommandJSONEntry::CommandJSONEntry(std::vector<uint8_t>& data) : in(std::make_unique<PImpl>())
{
	try {
		std::string str(data.begin(), data.end());
		json json_obj = json::parse(str);

		auto it = json_obj.begin();
		in->mNameInUTF8 = it.key();
		UTF2UTF(in->mNameInUTF8, in->mName);
		in->mData = it.value();
	}
	catch (const json::exception& e) {
		spdlog::error("Failed to parse response json: {}", e.what());
	}
}

CommandJSONEntry::~CommandJSONEntry()
{
}

void CommandJSONEntry::Init()
{
    in->mData = json::object();
}

// CommandEntryIF methods
LPCTSTR CommandJSONEntry::GetName()
{
    return in->mName;
}

void CommandJSONEntry::MarkAsUsed()
{
    in->mIsUsed = true;
}

bool CommandJSONEntry::IsUsedEntry()
{
    return in->mIsUsed;
}

bool CommandJSONEntry::HasValue(LPCTSTR key)
{
    std::string keyUtf8;
    UTF2UTF(key, keyUtf8);
    return in->mData.contains(keyUtf8);
}

int CommandJSONEntry::GetValueType(LPCTSTR key)
{
    std::string keyUtf8;
    UTF2UTF(key, keyUtf8);

    if (!in->mData.contains(keyUtf8)) {
        return VALUE_TYPE_NONE;
    }

    auto& value = in->mData[keyUtf8];
    if (value.contains("type")) {
        std::string typeStr = value["type"].get<std::string>();
        if (typeStr == "int") {
            return VALUE_TYPE_INT;
        } else if (typeStr == "double") {
            return VALUE_TYPE_DOUBLE;
        } else if (typeStr == "string") {
            return VALUE_TYPE_STRING;
        } else if (typeStr == "boolean") {
            return VALUE_TYPE_BOOL;
        } else if (typeStr == "bytes") {
            return VALUE_TYPE_BYTES;
        }
    }
    return VALUE_TYPE_NONE;
}

int CommandJSONEntry::Get(LPCTSTR key, int defValue)
{
    if (GetValueType(key) == VALUE_TYPE_INT) {
        std::string keyUtf8;
        UTF2UTF(key, keyUtf8);
        return in->mData[keyUtf8]["value"].get<int>();
    }
    return defValue;
}

void CommandJSONEntry::Set(LPCTSTR key, int value)
{
    std::string keyUtf8;
    UTF2UTF(key, keyUtf8);
    in->mData[keyUtf8]["type"] = "int";
    in->mData[keyUtf8]["value"] = value;
}

double CommandJSONEntry::Get(LPCTSTR key, double defValue)
{
    if (GetValueType(key) == VALUE_TYPE_DOUBLE) {
        std::string keyUtf8;
        UTF2UTF(key, keyUtf8);
        return in->mData[keyUtf8]["value"].get<double>();
    }
    return defValue;
}

void CommandJSONEntry::Set(LPCTSTR key, double value)
{
    std::string keyUtf8;
    UTF2UTF(key, keyUtf8);
    in->mData[keyUtf8]["type"] = "double";
    in->mData[keyUtf8]["value"] = value;
}

CString CommandJSONEntry::Get(LPCTSTR key, LPCTSTR defValue)
{
    if (GetValueType(key) == VALUE_TYPE_STRING) {
        std::string keyUtf8;
        UTF2UTF(key, keyUtf8);
        std::string valueUtf8 = in->mData[keyUtf8]["value"].get<std::string>();
        CString tmp;
        return UTF2UTF(valueUtf8, tmp);
    }
    return CString(defValue);
}

void CommandJSONEntry::Set(LPCTSTR key, const CString& value)
{
    std::string keyUtf8;
    UTF2UTF(key, keyUtf8);
    std::string valueUtf8;
    UTF2UTF(value, valueUtf8);

    in->mData[keyUtf8]["type"] = "string";
    in->mData[keyUtf8]["value"] = valueUtf8;
}

bool CommandJSONEntry::Get(LPCTSTR key, bool defValue)
{
    if (GetValueType(key) == VALUE_TYPE_BOOL) {
        std::string keyUtf8;
        UTF2UTF(key, keyUtf8);
        return in->mData[keyUtf8]["value"].get<bool>();
    }
    return defValue;
}

void CommandJSONEntry::Set(LPCTSTR key, bool value)
{
    std::string keyUtf8;
    UTF2UTF(key, keyUtf8);
    in->mData[keyUtf8]["type"] = "boolean";
    in->mData[keyUtf8]["value"] = value;
}

size_t CommandJSONEntry::GetBytesLength(LPCTSTR key)
{
    if (GetValueType(key) == VALUE_TYPE_BYTES) {
        std::string keyUtf8;
        UTF2UTF(key, keyUtf8);

        std::string encodedBytes = in->mData[keyUtf8]["value"].get<std::string>();
        CString tmp;
        UTF2UTF(encodedBytes, tmp);
        
        std::vector<uint8_t> decodedBytes;
        ::utility::base64::DecodeBase64(tmp, decodedBytes);
        return decodedBytes.size();
    }
    return 0;
}

bool CommandJSONEntry::GetBytes(LPCTSTR key, uint8_t* buf, size_t bufLen)
{
    if (GetValueType(key) == VALUE_TYPE_BYTES) {
        std::string keyUtf8;
        UTF2UTF(key, keyUtf8);

        std::string encodedBytes = in->mData[keyUtf8]["value"].get<std::string>();

        CString tmp;
        UTF2UTF(encodedBytes, tmp);

        std::vector<uint8_t> decodedBytes;
        if (::utility::base64::DecodeBase64(tmp, decodedBytes)) {
            if (decodedBytes.size() <= bufLen) {
                memcpy(buf, decodedBytes.data(), decodedBytes.size());
                return true;
            }
        }
    }
    return false;
}

void CommandJSONEntry::SetBytes(LPCTSTR key, const uint8_t* buf, size_t bufLen)
{
    std::string keyUtf8;
    UTF2UTF(key, keyUtf8);
    
    std::vector<uint8_t> bytes(buf, buf + bufLen);
    CString encodedBytes = ::utility::base64::EncodeBase64(bytes);

    std::string encodedBytesUtf8;
    UTF2UTF(encodedBytes, encodedBytesUtf8);

    in->mData[keyUtf8]["type"] = "bytes";
    in->mData[keyUtf8]["value"] = encodedBytesUtf8;
}

bool CommandJSONEntry::DumpRawData(std::vector<uint8_t>& rawData)
{
    auto json_data = json::object();
		json_data[in->mNameInUTF8] = in->mData;
    std::string s = json_data.dump();
    rawData.assign(s.begin(), s.end());
		rawData.push_back(0x00);
    return true;
}

uint32_t CommandJSONEntry::AddRef()
{
	return (uint32_t)InterlockedIncrement(&in->mRefCount);
}

uint32_t CommandJSONEntry::Release()
{
	auto n = InterlockedDecrement(&in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
}

} // end of namespace transfer
} // end of namespace commands
} // end of namespace launcherapp
