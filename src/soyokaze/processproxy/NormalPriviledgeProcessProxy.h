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
	// あふw側で選択中のファイル一覧を取得する
	bool GetAfxSelectionPath(std::wstring& path, int index);

	// エクスプローラのカレントディレクトリパスを取得する
	bool GetExplorerCurrentDir(std::wstring& path);
	// エクスプローラの選択要素のパスを取得する
	bool GetExplorerSelectionDir(std::wstring& path, int index);

	// Excelで現在選択中のワークシート等の情報を取得する
	bool GetExcelCurrentSelection(std::wstring& workbook, std::wstring& sheet, std::wstring& address);

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
	// オープンされているPowerpointスライドの一覧を取得する
	bool EnumPresentationSlides(std::wstring& filePath, std::vector<std::wstring>& slideTitles);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}  // end of namespace launcherapp::processproxy

