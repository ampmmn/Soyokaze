#pragma once

#include <cstddef>
#include <memory>
#include <string>

class NormalPriviledgeRequestBuffer
{
public:
	/**
	  コンストラクタ
	*/
	NormalPriviledgeRequestBuffer();
	/**
	  デストラクタ
	*/
	~NormalPriviledgeRequestBuffer();

	/**
	  名前付きパイプから読み取ったデータをバッファに追加する
	  @param[in] data 追加するデータ
	  @param[in] size データのサイズ
	*/
	void Append(const char* data, std::size_t size);

	/**
	  バッファから改行区切りのリクエストを1件取り出す
	  @param[out] request 取り出したリクエスト
	  @return true:リクエストを取り出した false:改行までのデータがない
	*/
	bool TryPopRequest(std::string& request);

	/**
	  バッファに未処理のデータが残っているか確認する
	  @return true:未処理データあり false:未処理データなし
	*/
	bool HasPendingData();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
