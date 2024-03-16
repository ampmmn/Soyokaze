#include "pch.h"
#include "ColorCommandProvider.h"
#include "commands/color/ColorCommand.h"
#include "commands/core/CommandRepository.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace color {


using CommandRepository = soyokaze::core::CommandRepository;

struct ColorCommandProvider::PImpl
{
	PImpl()
	{
	}
	virtual ~PImpl()
	{
	}

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(ColorCommandProvider)


ColorCommandProvider::ColorCommandProvider() : in(std::make_unique<PImpl>())
{
}

ColorCommandProvider::~ColorCommandProvider()
{
}

CString ColorCommandProvider::GetName()
{
	return _T("Color");
}

// 一時的なコマンドを必要に応じて提供する
void ColorCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	CString cmdline = pattern->GetWholeString();

	cmdline = cmdline.Trim();
	if (cmdline.IsEmpty()) {
		return;
	}

	static tregex regRGB(_T("^ *rgb *\\( *\\d+ *, *\\d+ *, *\\d+ *\\) *$"));
	static tregex regHSL(_T("^ *hsl *\\( *\\d+ *, *\\d+%? *, *\\d+%? *\\) *$"));

	if (cmdline[0] == _T('#') && cmdline.GetLength() == 7) {   // #rrggbb
		CString rrggbb = cmdline.Mid(1);

		uint32_t value;
		if (_stscanf_s(rrggbb, _T("%x"), &value) != 1) {
			return;
		}

		BYTE r = (value >> 16) & 0xFF;
		BYTE g = (value >>  8) & 0xFF;
		BYTE b = (value >>  0) & 0xFF;

		commands.push_back(CommandQueryItem(Pattern::WholeMatch, new ColorCommand(RGB(r, g, b), TYPE_HEX6)));
		commands.push_back(CommandQueryItem(Pattern::WholeMatch, new ColorCommand(RGB(r, g, b), TYPE_RGB)));
		commands.push_back(CommandQueryItem(Pattern::WholeMatch, new ColorCommand(RGB(r, g, b), TYPE_HSL)));
		return;
	}
	if (cmdline[0] == _T('#') && cmdline.GetLength() == 4) {   // #rgb
		CString rgb = cmdline.Mid(1);

		uint32_t value;
		if (_stscanf_s(rgb, _T("%x"), &value) != 1) {
			return;
		}

		BYTE r = ((value >> 8) & 0x0F) | ((value >> 4) & 0xF0);
		BYTE g = ((value >> 4) & 0x0F) | ((value >> 0) & 0xF0);
		BYTE b = ((value >> 0) & 0x0F) | ((value << 4) & 0xF0);

		commands.push_back(CommandQueryItem(Pattern::WholeMatch, new ColorCommand(RGB(r, g, b), TYPE_HEX3)));
		commands.push_back(CommandQueryItem(Pattern::WholeMatch, new ColorCommand(RGB(r, g, b), TYPE_HEX6)));
		commands.push_back(CommandQueryItem(Pattern::WholeMatch, new ColorCommand(RGB(r, g, b), TYPE_RGB)));
		commands.push_back(CommandQueryItem(Pattern::WholeMatch, new ColorCommand(RGB(r, g, b), TYPE_HSL)));
		return;
	}

	tstring cmdline_(cmdline);
	if (std::regex_match(cmdline_, regRGB)) {

		static tregex regParse(_T("^ *rgb *\\( *(\\d+) *, *(\\d+) *, *(\\d+) *\\) *$"));

		tstring rstr = std::regex_replace(cmdline_, regParse, _T("$1"));
		tstring gstr = std::regex_replace(cmdline_, regParse, _T("$2"));
		tstring bstr = std::regex_replace(cmdline_, regParse, _T("$3"));
		
		if (rstr.size() > 3 || gstr.size() > 3 || bstr.size() > 3) {
			return;
		}

		BYTE r = (BYTE)std::stoi(rstr);
		BYTE g = (BYTE)std::stoi(gstr);
		BYTE b = (BYTE)std::stoi(bstr);

		commands.push_back(CommandQueryItem(Pattern::WholeMatch, new ColorCommand(RGB(r, g, b), TYPE_RGB)));
		commands.push_back(CommandQueryItem(Pattern::WholeMatch, new ColorCommand(RGB(r, g, b), TYPE_HEX6)));
		commands.push_back(CommandQueryItem(Pattern::WholeMatch, new ColorCommand(RGB(r, g, b), TYPE_HSL)));
		return;
	}
}


} // end of namespace color
} // end of namespace commands
} // end of namespace soyokaze

