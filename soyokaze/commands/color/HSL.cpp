#include "pch.h"
#include "HSL.h"
#include <algorithm>
#include <cmath>

namespace soyokaze {
namespace commands {
namespace color {

HSL::HSL() : mH(0), mS(0), mL(0) 
{
}

HSL::~HSL()
{
}

void HSL::FromRGB(BYTE r, BYTE g, BYTE b)
{
	BYTE rgb[] = { r, g, b };

	auto maxElem = std::max_element(rgb, rgb+3);
	auto minElem = std::min_element(rgb, rgb+3);

	// Hue
	if (r == g && g == b && b == r) {
		mH = 0;
	}
	else if (minElem == rgb) { // R
		mH = (int)(std::round(60 * ((b - g) / (double)(*maxElem - *minElem)) + 180)) % 360;
	}
	else if (minElem == rgb + 1) { // G
		mH = (int)(std::round((60 * ((r - b) / (double)(*maxElem - *minElem))) + 300)) % 360;
	}
	else if (minElem == rgb + 2) { // B
		mH = (int)(std::round((60 * ((g - r) / (double)(*maxElem - *minElem))) + 60)) % 360;
	}

	// Lightness
	int cnt = (*maxElem + *minElem) / 2;
	mL = cnt / 255.0;

	// Saturation
	int denom = 255 - abs(*maxElem + *minElem - 255);
	if (denom != 0) {
		mS = (*maxElem - *minElem) / (double)denom;
	}
	else {
		mS = 0;
	}
}

}
}
}
