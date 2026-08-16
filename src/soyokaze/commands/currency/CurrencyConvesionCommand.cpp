// あ
#include "pch.h"
#include "framework.h"
#include "CurrencyConvesionCommand.h"
#include "icon/IconLoader.h"
#include "actions/clipboard/CopyClipboardAction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace currency {

struct CurrencyConvesionCommand::PImpl
{
	double mValue{0.0};
	CString mCurrency;
	CString mValueText;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(CurrencyConvesionCommand)

CurrencyConvesionCommand::CurrencyConvesionCommand(double value, const CString& currency) : in(std::make_unique<PImpl>())
{
	in->mValue = value;
	in->mCurrency = currency;
	in->mValueText.Format(_T("%.2f"), value);
	mName = in->mValueText + _T(" ") + in->mCurrency;
	mDescription = mName;
}

CurrencyConvesionCommand::~CurrencyConvesionCommand()
{
}

CString CurrencyConvesionCommand::GetName()
{
	return mName;
}

CString CurrencyConvesionCommand::GetDescription()
{
	return mDescription;
}

CString CurrencyConvesionCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool CurrencyConvesionCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() != 0) {
		return false;
	}

	*action = new actions::clipboard::CopyTextAction(in->mValueText);
	return true;
}

HICON CurrencyConvesionCommand::GetIcon()
{
	return IconLoader::Get()->GetShell32Icon(-16801);
}


launcherapp::core::Command* CurrencyConvesionCommand::Clone()
{
	return new CurrencyConvesionCommand(in->mValue, in->mCurrency);
}

CString CurrencyConvesionCommand::TypeDisplayName()
{
	return _T("通貨変換");
}

}}}
