#include "ShellExecuteProxyCommand.h"
#include <windows.h>
#include <map>
#include "StringUtil.h"
#include <servprov.h>
#include <shobjidl_core.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

/**
 * @brief 環境変数を設定するためのクラス
 */
struct AdditionalEnvVariableSite : 
	winrt::implements<AdditionalEnvVariableSite, ::IServiceProvider, ::ICreatingProcess>
{
public:

	/**
	 * @brief 環境変数を設定する
	 * @param vals 環境変数のキーと値のマップ
	 */
	void SetEnvironmentVariables(const std::map<std::wstring, std::wstring>& vals) {
		mEnvMap = vals;
	}

	/**
	 * @brief サービスをクエリする
	 * @param service サービスの GUID
	 * @param riid インターフェース ID
	 * @param ppv インターフェースへのポインタ
	 * @return 実装されていない場合は E_NOTIMPL
	 */
	IFACEMETHOD(QueryService)(REFGUID service, REFIID riid, void** ppv) {
		if (service != SID_ExecuteCreatingProcess) {
			*ppv = nullptr;
			return E_NOTIMPL;
		}
		return this->QueryInterface(riid, ppv);
	}

	/**
	 * @brief プロセス作成時に呼び出される
	 * @param inputs プロセス作成の入力
	 * @return 成功時は S_OK
	 */
	IFACEMETHOD(OnCreating)(ICreateProcessInputs* inputs) {
		for (auto& item : mEnvMap) {
			HRESULT hr = inputs->SetEnvironmentVariable(item.first.c_str(), item.second.c_str());
			if (hr != S_OK) {
				spdlog::error("SetEnvironmentVariable failed: {:x}", hr);
				break;
			}
		}
		return S_OK;
	}

private:
	std::map<std::wstring, std::wstring> mEnvMap; ///< 環境変数のマップ
};


namespace launcherproxy { 

REGISTER_PROXYCOMMAND(ShellExecuteProxyCommand)

ShellExecuteProxyCommand::ShellExecuteProxyCommand()
{
}

ShellExecuteProxyCommand::~ShellExecuteProxyCommand()
{
}

std::string ShellExecuteProxyCommand::GetName()
{
	return "shellexecute";
}

bool ShellExecuteProxyCommand::Execute(json& json_req, json& json_res)
{
	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = json_req["show_type"];
	si.fMask = json_req["mask"];

	// 実行ファイルのパスを設定
	std::wstring file;
	si.lpFile = utf2utf(json_req["file"], file).c_str();

	// パラメータを設定
	std::wstring parameters;
	if (json_req.find("parameters") != json_req.end()) {
		std::string src = json_req["parameters"];
		si.lpParameters = utf2utf(src, parameters).c_str();
	}

	// 作業ディレクトリを設定
	std::wstring directory;
	if (json_req.find("directory") != json_req.end()) {
		std::string src = json_req["directory"];
		si.lpDirectory = utf2utf(src, directory).c_str();
	}

	// 環境変数を設定
	std::map<std::wstring, std::wstring> env_map;
	if (json_req.find("environment") != json_req.end()) {
		std::wstring dst_key;
		std::wstring dst_value;
		auto dict = json_req.find("environment");
		for (auto it = dict->begin(); it != dict->end(); ++it) {
			env_map[utf2utf(it.key(), dst_key)] = utf2utf(it.value(), dst_value);
		}
	}

	// 追加の環境変数が設定されている場合
	auto site = winrt::make_self<AdditionalEnvVariableSite>();
	site->SetEnvironmentVariables(env_map);
	if (env_map.empty() == false) {
		si.fMask |= SEE_MASK_FLAG_HINST_IS_SITE;
		si.hInstApp = reinterpret_cast<HINSTANCE>(site.get());
	}

	// ShellExecuteEx を実行
	BOOL isRun = ShellExecuteEx(&si);

	// 起動したプロセス ID を取得
	DWORD pid = 0xFFFFFFFF;
	if (si.hProcess) {
		pid = GetProcessId(si.hProcess);
		CloseHandle(si.hProcess);
		si.hProcess = nullptr;
	}

	// 結果を JSON 形式で親プロセスに返す
	json_res["result"] = isRun != FALSE;
	json_res["pid"] = (int)pid;

	return true;
}

} // end of namespace 



