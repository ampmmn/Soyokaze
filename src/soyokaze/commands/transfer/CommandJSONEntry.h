#pragma once

#include "commands/core/CommandEntryIF.h"
#include "utility/RefPtr.h"
#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace transfer {

class CommandJSONEntry : public CommandEntryIF
{
public:
    CommandJSONEntry(LPCTSTR name);
    CommandJSONEntry(std::vector<uint8_t>& data);
    ~CommandJSONEntry();

    /**
      複数のコマンドエントリをクリップボード用データに変換する
      @param[in] entries コマンドエントリの一覧
      @param[out] data 変換後のデータ
      @return true:成功 false:失敗
    */
    static bool PackEntries(const std::vector<CommandEntryIF*>& entries, std::vector<uint8_t>& data);

    /**
      クリップボード用データから複数のコマンドエントリを生成する
      @param[in] data クリップボード用データ
      @param[out] entries 生成したコマンドエントリの一覧
      @return true:成功 false:失敗
    */
    static bool UnpackEntries(const std::vector<uint8_t>& data, std::vector<RefPtr<CommandEntryIF>>& entries);

    /**
     * @brief オブジェクトを初期化する
     */
    void Init();

    // CommandEntryIFのインターフェース
public:
    LPCTSTR GetName() override;
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
    size_t GetBytesLength(LPCTSTR key) override;
    bool GetBytes(LPCTSTR key, uint8_t* buf, size_t bufLen) override;
    void SetBytes(LPCTSTR key, const uint8_t* buf, size_t bufLen) override;
    bool DumpRawData(std::vector<uint8_t>& rawData) override;
		uint32_t AddRef() override;
		uint32_t Release() override;

public:
    /**
     *  @brief 値を取得する際の型
     */
    enum ValueType {
        VALUE_TYPE_NONE = 0,
        VALUE_TYPE_INT,
        VALUE_TYPE_DOUBLE,
        VALUE_TYPE_STRING,
        VALUE_TYPE_BOOL,
        VALUE_TYPE_BYTES,
    };

private:
    struct PImpl;
    std::unique_ptr<PImpl> in;
};

} // end of namespace transfer
} // end of namespace commands
} // end of namespace launcherapp
