#include "pch.h"
#include "TimespanCommandProvider.h"
#include "commands/timespan/TimespanCommand.h"
#include "commands/core/CommandRepository.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace timespan {


using CommandRepository = soyokaze::core::CommandRepository;

struct TimespanCommandProvider::PImpl
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

REGISTER_COMMANDPROVIDER(TimespanCommandProvider)


TimespanCommandProvider::TimespanCommandProvider() : in(std::make_unique<PImpl>())
{
}

TimespanCommandProvider::~TimespanCommandProvider()
{
}

CString TimespanCommandProvider::GetName()
{
	return _T("Timespan");
}

// 一時的なコマンドを必要に応じて提供する
void TimespanCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	CString cmdline = pattern->GetWholeString();

	cmdline = cmdline.Trim();
	if (cmdline.IsEmpty()) {
		return;
	}

	tstring cmdline_(cmdline);

	static tregex regTime1(_T("^ *([0-2][0-9]):([0-5][0-9]) *- *([0-2][0-9]):([0-5][0-9]) *$"));
	if (std::regex_match(cmdline_, regTime1)) {

		auto h1 = (uint8_t)std::stoi(std::regex_replace(cmdline_, regTime1, _T("$1")));
		if (h1 >= 24) {
			return;
		}
		auto m1 = (uint8_t)std::stoi(std::regex_replace(cmdline_, regTime1, _T("$2")));
		auto h2 = (uint8_t)std::stoi(std::regex_replace(cmdline_, regTime1, _T("$3")));
		if (h2 >= 24) {
			return;
		}
		auto m2 = (uint8_t)std::stoi(std::regex_replace(cmdline_, regTime1, _T("$4")));

		CTimeSpan ts1(0, h1, m1, 0);
		CTimeSpan ts2(0, h2, m2, 0);

		CTimeSpan result = ts1 - ts2;
		commands.push_back(CommandQueryItem(Pattern::WholeMatch, new TimespanCommand(result, TYPE_HOUR)));
		commands.push_back(CommandQueryItem(Pattern::WholeMatch, new TimespanCommand(result, TYPE_MINUTE)));
		commands.push_back(CommandQueryItem(Pattern::WholeMatch, new TimespanCommand(result, TYPE_SECOND)));
		return;
	}
}


} // end of namespace timespan
} // end of namespace commands
} // end of namespace soyokaze

