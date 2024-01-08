#include "pch.h"
#include "framework.h"
#include "SimpleDictAdhocCommand.h"
#include "commands/common/Clipboard.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace soyokaze::commands::common;

namespace soyokaze {
namespace commands {
namespace simple_dict {

struct SimpleDictAdhocCommand::PImpl
{
	CString mRecord;    // ブラウザ種類を表す文字列
};


SimpleDictAdhocCommand::SimpleDictAdhocCommand(
	const CString& record
) : 
	AdhocCommandBase(record, record),
	in(std::make_unique<PImpl>())
{
	in->mRecord = record;
}

SimpleDictAdhocCommand::~SimpleDictAdhocCommand()
{
}

CString SimpleDictAdhocCommand::GetGuideString()
{
	return _T("Enter:クリップボードにコピー");
}

CString SimpleDictAdhocCommand::GetTypeDisplayName()
{
	return _T("簡易辞書");
}

BOOL SimpleDictAdhocCommand::Execute(const Parameter& param)
{
	// URLをクリップボードにコピー
	Clipboard::Copy(in->mRecord);
	return TRUE;
}

HICON SimpleDictAdhocCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-5301);
}

soyokaze::core::Command*
SimpleDictAdhocCommand::Clone()
{
	return new SimpleDictAdhocCommand(in->mRecord);
}

} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace soyokaze

