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


private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}
}

