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

