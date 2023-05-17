#pragma once

// コマンド実行履歴
class ExecHistory
{
public:
	ExecHistory();
	~ExecHistory();

public:
	// 実行したコマンド文字列を追加
	void Add(const CString& commandStr);

	// 指定したコマンド文字列が前回実行されたかを表す数値を取得する
	size_t GetOrder(const CString& commandStr);

	// 
	void SetLimit(int limit);

	void Load();
	void Save();

protected:
	struct PImpl;
	PImpl* in;
};

