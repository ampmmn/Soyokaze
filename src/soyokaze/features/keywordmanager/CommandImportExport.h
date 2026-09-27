#pragma once

#include <afxstr.h>
#include <memory>
#include <vector>

namespace launcherapp { namespace core {
class Command;
}}

/**
  コマンドのインポートとエクスポートを行う
*/
class CommandImportExport
{
public:
	enum class ExportError
	{
		None,
		CommandSaveFailed,
		FileSaveFailed,
	};

	struct ExportResult
	{
		ExportError mError{ExportError::None};
		CString mCommandName;
	};

public:
	CommandImportExport();
	~CommandImportExport();

	/**
	  インポートファイルを読み込み、対応するコマンドを構築する
	  @param[in] filePath インポートファイルのパス
	  @return true:成功 false:失敗
	*/
	bool LoadCommands(const CString& filePath);

	/**
	  読み込んだインポート候補を取得する
	  @return CommandImportExportのインスタンスが有効な間だけ参照可能なコマンド一覧
	*/
	const std::vector<launcherapp::core::Command*>& GetImportCandidates() const;

	/**
	  読み込めなかったエントリ名を取得する
	  @return 読み込めなかったエントリ名一覧
	*/
	const std::vector<CString>& GetSkippedEntryNames() const;

	/**
	  選択されたコマンドをリポジトリに登録する
	  @param[in] selectedIndices インポートする候補のインデックス一覧
	  @param[in] isOverwriteSelected 名前が重複した場合に上書きするか
	  @return インポートしたコマンド名一覧
	*/
	std::vector<CString> ImportCommands(const std::vector<int>& selectedIndices, bool isOverwriteSelected);

	/**
	  指定されたコマンドをファイルにエクスポートする
	  @param[in] commands エクスポートするコマンド一覧
	  @param[in] filePath エクスポート先ファイルのパス
	  @return エクスポート結果
	*/
	ExportResult ExportCommands(const std::vector<launcherapp::core::Command*>& commands, const CString& filePath);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
