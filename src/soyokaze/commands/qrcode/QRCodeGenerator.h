#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace qrcode {

/**
  QRコード画像を生成し、コピーやファイル保存を行う
*/
class QRCodeGenerator
{
public:
	/**
	  誤り訂正レベル
	*/
	enum ErrorCorrectionLevel
	{
		LEVEL_L = 0,
		LEVEL_M,
		LEVEL_Q,
		LEVEL_H,
	};

	/**
	  ジェネレーターを初期化する
	*/
	QRCodeGenerator();
	/**
	  生成画像などのリソースを解放する
	*/
	~QRCodeGenerator();

	/**
	  指定した文字列からQRコードを生成する
	  @return true:成功  false:失敗または容量超過
	  @param[in] text QRコードにする文字列
	  @param[in] level 誤り訂正レベル
	*/
	bool Generate(const CString& text, int level);

	/**
	  生成した画像をクリップボードへコピーする
	  @return true:成功  false:失敗
	*/
	bool CopyToClipboard() const;

	/**
	  生成した画像をファイルへ保存する
	  @return true:成功  false:失敗
	  @param[in] path 保存先のパス
	*/
	bool SaveImage(LPCTSTR path) const;

	/**
	  生成したQRコードをSVG形式で保存する
	  @return true:成功  false:失敗
	  @param[in] path 保存先のパス
	*/
	bool SaveSvg(LPCTSTR path) const;

	/**
	  生成したQRコード画像を指定領域へ描画する
	  @param[in] dc 描画先のデバイスコンテキスト
	  @param[in] rect 描画領域
	*/
	void Draw(HDC dc, const RECT& rect) const;

	/**
	  有効なQRコード画像を保持しているかを取得する
	  @return true:保持している  false:保持していない
	*/
	bool IsValid() const;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // qrcode名前空間
} // commands名前空間
} // launcherapp名前空間
