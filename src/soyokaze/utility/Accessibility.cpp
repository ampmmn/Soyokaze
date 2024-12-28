#include "pch.h"
#include "Accessibility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace utility {

bool IsHighContrastMode()
{
	HIGHCONTRAST hc;
	hc.cbSize = sizeof(HIGHCONTRAST);
	SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRAST), &hc, 0);
	return (hc.dwFlags & HCF_HIGHCONTRASTON) != 0;
}

}

