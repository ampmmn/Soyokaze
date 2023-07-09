#pragma once

namespace soyokaze {
namespace commands {
namespace calculator {


class Calculator
{
public:
	Calculator();
	~Calculator();

public:
	// python.dllのパスを設定する
	void SetPythonDLLPath(const CString& dllPath);

	// 式を評価する
	bool Evaluate(const CString& src, CString& result);


private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}
}

