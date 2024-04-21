#pragma once

namespace launcherapp {
namespace commands {
namespace color {

class HSL
{
public:
	HSL();
	~HSL();

	void FromRGB(BYTE r, BYTE g, BYTE b);
	
	int H() const { return mH; }
	double S() const { return mS; }
	double L() const { return mL; }

private:
	// 色相(0～360)
	int mH;
	// 彩度(0～100%)
	double mS;
	// 明度(0～100%)
	double mL;
};

}
}
}
