#pragma once

namespace launcherapp {
namespace commands {
namespace calculator {


class Calculator
{
public:
	Calculator();
	~Calculator();

public:
	// 式を評価する
	bool Evaluate(const CString& src, CString& result);

	/**
	  電卓機能として評価してよい式かどうかを検証する
	  四則演算とmathモジュールの関数・定数のみを許可し、
	  任意のコードを実行できる式を拒否する
	  @return true:評価してよい false:評価しない
	  @param[in] src 検証する式
	*/
	static bool IsValidExpression(const CString& src);

	/**
	  標準電卓機能の有効状態を設定する
	  @param[in] use 有効にする場合はtrue
	*/
	void UseStandardEvaluate(bool use);

	/**
	  MathyPadによる単位変換機能の有効状態を設定する
	  @param[in] use 有効にする場合はtrue
	*/
	void UseMathypadEvaluate(bool use);

	bool IsUseStandardEvaluate();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}
}

