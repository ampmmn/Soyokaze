#pragma once

#include "gui/ColorScheme.h"

class ColorSettings
{
private:
	ColorSettings();
	~ColorSettings();

public:
	static ColorSettings* Get();

	bool IsSystemSettings();

	ColorSchemeIF* GetCurrentScheme();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

