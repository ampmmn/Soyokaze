#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace plugin {

class PluginSettings
{
public:
	struct Item
	{
		bool mIsEnabled{true};
		int mPriority{0};
	};

	PluginSettings();
	~PluginSettings();

	/**
	  プラグイン設定の共有インスタンスを取得する
	  @return 共有インスタンス
	*/
	static PluginSettings* GetInstance();
	/**
	  プラグイン設定をファイルから読み込む
	  @return true:成功 false:失敗
	*/
	bool Load();
	/**
	  プラグイン設定をファイルへ保存する
	  @return true:成功 false:失敗
	*/
	bool Save() const;

	/**
	  プラグインIDに対応する設定を取得する
	  @param[in] pluginId プラグインID
	  @return プラグイン設定
	*/
	Item Get(const CString& pluginId) const;
	/**
	  プラグイン設定を更新する
	  @param[in] pluginId プラグインID
	  @param[in] item 更新する設定
	*/
	void Set(const CString& pluginId, const Item& item);

	/**
	  プラグイン設定の複製を作成する
	  @return 複製された設定
	*/
	std::unique_ptr<PluginSettings> Clone() const;
	/**
	  保持している設定を指定先へコピーする
	  @param[out] destination コピー先
	*/
	void CopyTo(PluginSettings* destination) const;
	/**
	  設定ファイルのパスを指定する
	  @param[in] path 設定ファイルのパス
	*/
	void SetFilePath(const CString& path);

	private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // namespace plugin
} // namespace commands
} // namespace launcherapp
