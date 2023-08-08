#include "pch.h"
#include "ColorCommandProvider.h"
#include "commands/color/ColorCommand.h"
#include "core/CommandRepository.h"

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


ColorCommandProvider::ColorCommandProvider() : in(new PImpl)
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
 	std::vector<CommandQueryItem>& commands
)
{
	CString cmdline = pattern->GetWholeString();

	cmdline = cmdline.Trim();
	ASSERT(cmdline.IsEmpty() == FALSE);

	if (cmdline[0] == _T('#') && cmdline.GetLength() == 7) {   // #rrggbb
		CString rrggbb = cmdline.Mid(1);

		uint32_t value;
		if (_stscanf_s(rrggbb, _T("%x"), &value) != 1) {
			return;
		}

		BYTE r = (value >> 16) & 0xFF;
		BYTE g = (value >>  8) & 0xFF;
		BYTE b = (value >>  0) & 0xFF;

		auto cmd = new ColorCommand(RGB(r, g, b));
		commands.push_back(CommandQueryItem(Pattern::WholeMatch, cmd));
	}
	else if (cmdline[0] == _T('#') && cmdline.GetLength() == 4) {   // #rgb
		CString rgb = cmdline.Mid(1);

		uint32_t value;
		if (_stscanf_s(rgb, _T("%x"), &value) != 1) {
			return;
		}

		BYTE r = ((value >> 8) & 0x0F) | ((value >> 4) & 0xF0);
		BYTE g = ((value >> 4) & 0x0F) | ((value >> 0) & 0xF0);
		BYTE b = ((value >> 0) & 0x0F) | ((value << 4) & 0xF0);

		auto cmd = new ColorCommand(RGB(r, g, b));
		commands.push_back(CommandQueryItem(Pattern::WholeMatch, cmd));
	}
}


} // end of namespace color
} // end of namespace commands
} // end of namespace soyokaze

