#pragma once

#include <cstdint>
#include <memory>
#include <map>
#include <string>
#include <vector>

namespace launcherapp { namespace processproxy {

class NormalPriviledgeProcessProxy
{
	NormalPriviledgeProcessProxy();
	~NormalPriviledgeProcessProxy();

public:
	static NormalPriviledgeProcessProxy* GetInstance();

	// 通常権限でコマンドを実行する
	bool StartProcess(SHELLEXECUTEINFO* si, const std::map<std::wstring, std::wstring>& envMap);
	// あふwのカレントディレクトリを取得する
	bool GetCurrentAfxwDir(std::wstring& path);
	// あふwのカレントディレクトリを設定する
	bool SetCurrentAfxwDir(const std::wstring& path);

	// オープンされているExcelのシート一覧を取得する
	bool EnumExcelSheets(std::vector<std::pair<std::wstring, std::wstring> >& sheets);
	// Excelシートをアクティブにする
	bool ActiveExcelSheet(const std::wstring& workbook, const std::wstring& worksheet, bool isShowMaximize);
	// オープンされているCalcのシート一覧を取得する
	bool EnumCalcSheets(std::vector<std::pair<std::wstring, std::wstring> >& sheets);
	// Calcシートをアクティブにする
	bool ActiveCalcSheet(const std::wstring& workbook, const std::wstring& worksheet, bool isShowMaximize);

	// アクティブなPowerpointのウインドウを取得する
	bool GetActivePowerPointWindow(HWND& hwnd);
	// アクティブなPowerpoint上の表示スライドを指定ページに移動する
	bool GoToSlide(int16_t pageIndex);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}  // end of namespace launcherapp::processproxy

