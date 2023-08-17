#pragma once

#include "core/CommandIF.h"
#include "core/CommandQueryItem.h"
#include "Pattern.h"
#include <stdint.h>

class CommandFile;

namespace soyokaze {
namespace core {

class CommandParameter;

class CommandProvider
{
public:
	using CommandQueryItemList = soyokaze::CommandQueryItemList;

public:
	virtual ~CommandProvider() {}

	// 初回起動の初期化を行う
	virtual void OnFirstBoot() = 0;

	// コマンドの読み込み
	virtual void LoadCommands(CommandFile* commandFile) = 0;

	// 作成できるコマンドの種類を識別するための文字列を取得
	virtual CString GetName() = 0;

	// 作成できるコマンドの種類を表す表示用の文字列を取得
	virtual CString GetDisplayName() = 0;

	// コマンドの種類の説明を示す文字列を取得
	virtual CString GetDescription() = 0;

	// コマンド新規作成ダイアログを表示する
	virtual bool NewDialog(const CommandParameter* param = nullptr) = 0;

	// 非公開コマンドかどうか(新規作成対象にしない)
	virtual bool IsPrivate() const = 0;

	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) = 0;

	// Provider間の優先順位を表す値を返す。小さいほど優先
	virtual uint32_t GetOrder() const = 0;

	virtual uint32_t AddRef() = 0;
	virtual uint32_t Release() = 0;

};

}
} // end of namespace soyokaze::core

// 派生クラス側で下記のマクロを通じてCommandProviderとして登録する

#define DECLARE_COMMANDPROVIDER(clsName)   static bool RegisterCommandProvider(); \
	private: \
	static bool _mIsRegistered; \
	public: \
	static bool IsRegistered();

#define REGISTER_COMMANDPROVIDER(clsName)   \
	bool clsName::RegisterCommandProvider() { \
		try { \
			clsName* inst = new clsName(); \
			soyokaze::core::CommandRepository::GetInstance()->RegisterProvider(inst); \
			inst->Release(); \
			return true; \
		} catch(...) { return false; } \
	} \
	bool clsName::_mIsRegistered = clsName::RegisterCommandProvider(); \
	bool clsName::IsRegistered() { return _mIsRegistered; }

