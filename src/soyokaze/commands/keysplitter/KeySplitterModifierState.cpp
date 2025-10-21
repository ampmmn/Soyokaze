#include "pch.h"
#include "KeySplitterModifierState.h"

namespace launcherapp {
namespace commands {
namespace keysplitter {

ModifierState::ModifierState() 
{
}

ModifierState::ModifierState(int stateBits)  : 
	mIsPressShift((stateBits & SHIFT) != 0),
	mIsPressCtrl((stateBits & CONTROL) != 0),
	mIsPressAlt((stateBits & ALT) != 0),
	mIsPressWin((stateBits & WIN) != 0)
{
}

ModifierState::ModifierState(bool isPressShift, bool isPressCtrl, bool isPressAlt, bool isPressWin) : 
	mIsPressShift(isPressShift),
	mIsPressCtrl(isPressCtrl),
	mIsPressAlt(isPressAlt),
	mIsPressWin(isPressWin)
{
	SetPressShift(isPressShift);
	SetPressCtrl(isPressCtrl);
	SetPressAlt(isPressAlt);
	SetPressWin(isPressWin);
}

ModifierState::ModifierState(const ModifierState& rhs) : 
	mIsPressShift(rhs.mIsPressShift),
	mIsPressCtrl(rhs.mIsPressCtrl),
	mIsPressAlt(rhs.mIsPressAlt),
	mIsPressWin(rhs.mIsPressWin)
{
}

bool ModifierState::operator == (const ModifierState& rhs) const
{
	return ToBits() == rhs.ToBits();
}

bool ModifierState::operator != (const ModifierState& rhs) const
{
	return ToBits() != rhs.ToBits();
}

bool ModifierState::operator < (const ModifierState& rhs) const
{
	return ToBits() < rhs.ToBits();
}

bool ModifierState::IsPressShift() const
{
 	return mIsPressShift;
}

void ModifierState::SetPressShift(bool isPressShift)
{
 	mIsPressShift= isPressShift;
}

bool ModifierState::IsPressCtrl() const
{
 	return mIsPressCtrl;
}

void ModifierState::SetPressCtrl(bool isPressCtrl)
{
 	mIsPressCtrl= isPressCtrl;
}

bool ModifierState::IsPressAlt() const
{
 	return mIsPressAlt;
}

void ModifierState::SetPressAlt(bool isPressAlt)
{
 	mIsPressAlt= isPressAlt;
}

bool ModifierState::IsPressWin() const
{
 	return mIsPressWin;
}

void ModifierState::SetPressWin(bool isPressWin)
{
 	mIsPressWin= isPressWin;
}

CString ModifierState::ToString() const
{
	auto f = [](bool state, LPCTSTR text) -> CString {
		CString tmp;
		if (state) {
			tmp += text;
			tmp += _T("-");
		}
		return tmp;
	};

	CString ret;
	ret += f(IsPressShift(), _T("S"));
	ret += f(IsPressCtrl(), _T("C"));
	ret += f(IsPressAlt(), _T("A"));
	ret += f(IsPressWin(), _T("W"));
	ret += _T("⏎");
	return ret;
}

int ModifierState::ToBits() const
{
	int state = 0;
	state |= (IsPressShift() ? SHIFT : 0);
	state |= (IsPressCtrl() ? CONTROL : 0);
	state |= (IsPressAlt() ? ALT : 0);
	state |= (IsPressWin() ? WIN : 0);
	return state;
}

} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp


