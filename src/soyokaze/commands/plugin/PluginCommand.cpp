// あ
#include "pch.h"
#include "PluginCommand.h"
#include "actions/core/ActionBase.h"
#include "icon/IconLoader.h"
#include "utility/CharConverter.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace plugin {

namespace {

/**
  プラグインAPIから取得した文字列をCStringへ変換する
  @param[in] func 文字列取得関数
  @param[in] match プラグインの検索結果ハンドル
  @param[in] index 検索結果内のコマンド番号
  @param[out] value 取得した文字列
  @return true:取得成功 false:取得失敗
*/
bool GetStringValue(LNCRPLUGINFUNC_GETSTRING func, LNCRPLUGINMATCHHANDLE match,
	int index, CString& value)
{
	value.Empty();
	// まず必要なバッファサイズを問い合わせてから、文字列本体を取得する。
	int len = func(match, index, nullptr, 0);
	if (len < 0) {
		return false;
	}
	if (len == 0) {
		return true;
	}

	std::vector<char> buffer(static_cast<size_t>(len), '\0');
	if (func(match, index, buffer.data(), buffer.size()) < 0) {
		return false;
	}
	buffer.back() = '\0';

	std::string utf8(buffer.data());
	UTF2UTF(utf8, value);
	return true;
}

class PluginAction : public launcherapp::actions::core::ActionBase
{
public:
	/**
	  プラグインコマンド実行用アクションを生成する
	  @param[in] module プラグインモジュール
	  @param[in] match プラグインの検索結果ハンドル
	  @param[in] index 検索結果内のコマンド番号
	  @param[in] displayName アクションの表示名
	*/
	PluginAction(const PluginModulePtr& module, const PluginMatchPtr& match, int index,
	            const CString& displayName) :
		mModule(module), mMatch(match), mIndex(index), mDisplayName(displayName)
	{
	}

	/**
	  アクションの表示名を取得する
	  @return アクションの表示名
	*/
	CString GetDisplayName() override
	{
		return mDisplayName;
	}

	/**
	  プラグインへ実行時引数を渡してコマンドを実行する
	  @param[in] param 実行時パラメータ
	  @param[out] errMsg 実行失敗時のエラーメッセージ
	  @return true:実行成功 false:実行失敗
	*/
	bool Perform(Parameter* param, String* errMsg) override
	{
		std::vector<std::string> arguments;
		std::vector<char*> argv;
		int argc = param ? param->GetParamCount() : 0;
		arguments.reserve(argc);
		argv.reserve(argc + 1);

		// ランチャー側の文字列をプラグインAPI用のUTF-8文字列へ変換する。
		for (int i = 0; i < argc; ++i) {
			std::string argument;
			UTF2UTF(std::wstring(param->GetParam(i)), argument);
			arguments.push_back(std::move(argument));
		}
		for (auto& argument : arguments) {
			argv.push_back(argument.data());
		}
		argv.push_back(nullptr);

		// Executeにはコマンド名を含めず、実行時引数だけを渡す。
		int result = mModule->mExportTable.Execute(
			static_cast<LNCRPLUGINMATCHHANDLE>(mMatch.get()), mIndex, argc, argv.data());
		if (result == 0) {
			return true;
		}

		// 実行に失敗した場合は、プラグインからエラー内容を取得する。
		if (errMsg) {
			CString error;
			if (GetStringValue(mModule->mExportTable.GetErrorString,
				static_cast<LNCRPLUGINMATCHHANDLE>(mMatch.get()), mIndex, error)) {
				std::string errorUtf8;
				UTF2UTF(std::wstring(error), errorUtf8);
				*errMsg = errorUtf8;
			}
		}
		return false;
	}

private:
	PluginModulePtr mModule;
	PluginMatchPtr mMatch;
	int mIndex;
	CString mDisplayName;
};

} // namespace

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(PluginCommand)

PluginCommand::PluginCommand(const PluginModulePtr& module, const PluginMatchPtr& match,
	int index, const CString& name, const CString& description) :
	AdhocCommandBase(name, description), mModule(module), mMatch(match), mIndex(index)
{
	// コマンド種別は生成時に取得し、以後はコマンド側で保持する。
	GetStringValue(mModule->mExportTable.GetTypeDisplayName,
		static_cast<LNCRPLUGINMATCHHANDLE>(mMatch.get()), mIndex, mTypeDisplayName);
}

PluginCommand::~PluginCommand()
{
}

/**
  コマンド種別の表示名を取得する
  @return コマンド種別の表示名
*/
CString PluginCommand::GetTypeDisplayName()
{
	return mTypeDisplayName;
}

/**
  プラグイン側の判定結果に基づいて実行可否を返す
  @param[out] reasonMsg 実行できない場合の理由
  @return true:実行可能 false:実行不可
*/
bool PluginCommand::CanExecute(String* reasonMsg)
{
	int result = mModule->mExportTable.CanExecute(static_cast<LNCRPLUGINMATCHHANDLE>(mMatch.get()), mIndex);
	if (result != 0) {
		return true;
	}

	// 実行不可の場合だけ、プラグインが提供する理由を取得する。
	if (reasonMsg) {
		CString reason;
		if (GetStringValue(mModule->mExportTable.GetErrorString,
			static_cast<LNCRPLUGINMATCHHANDLE>(mMatch.get()), mIndex, reason)) {
			std::string reasonUtf8;
			UTF2UTF(std::wstring(reason), reasonUtf8);
			*reasonMsg = reasonUtf8;
		}
	}
	return false;
}

/**
  ホットキー属性を確認してプラグインアクションを生成する
  @param[in] hotkeyAttr 起動時に指定されたホットキー属性
  @param[out] action 生成したアクション
  @return true:生成成功 false:生成不可
*/
bool PluginCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	// 修飾キー付き起動と出力先不在の場合はアクションを生成しない。
	if (hotkeyAttr.GetModifiers() != 0 || action == nullptr) {
		return false;
	}

	CString guide;
	if (GetStringValue(mModule->mExportTable.GetGuide,
		static_cast<LNCRPLUGINMATCHHANDLE>(mMatch.get()), mIndex, guide) == false) {
		return false;
	}

	*action = new PluginAction(mModule, mMatch, mIndex, guide);
	return true;
}

/**
  プラグインからアイコンを取得し、失敗時は標準アイコンを返す
  @return アイコンハンドル
*/
HICON PluginCommand::GetIcon()
{
	// アイコン取得は初回だけ行い、以後は取得済みのハンドルを再利用する。
	if (mIcon) {
		return mIcon;
	}

	if (mModule->mExportTable.GetIcon(
		static_cast<LNCRPLUGINMATCHHANDLE>(mMatch.get()), mIndex, &mIcon) != 0 ||
		mIcon == nullptr) {
		mIcon = IconLoader::Get()->LoadDefaultIcon();
	}
	return mIcon;
}

/**
  現在のプラグインコマンドを複製する
  @return 複製されたコマンド
*/
launcherapp::core::Command* PluginCommand::Clone()
{
	return new PluginCommand(mModule, mMatch, mIndex, mName, mDescription);
}

} // namespace plugin
} // namespace commands
} // namespace launcherapp
