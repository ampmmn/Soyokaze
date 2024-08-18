#include "pch.h"
#include "KeySplitterModifierState.h"

namespace launcherapp {
namespace commands {
namespace keysplitter {

ModifierState::ModifierState() : mStateBits(0) {}

ModifierState::ModifierState(int stateBits) : mStateBits(stateBits) {}

ModifierState::ModifierState(bool isPressShift, bool isPressCtrl, bool isPressAlt, bool isPressWin) : 
	mStateBits(0)
{
	SetPressShift(isPressShift);
	SetPressCtrl(isPressCtrl);
	SetPressAlt(isPressAlt);
	SetPressWin(isPressWin);
}

ModifierState::ModifierState(const ModifierState& rhs) : mStateBits(rhs.mStateBits) {}

bool ModifierState::operator < (const ModifierState& rhs) const
{
	return mStateBits < rhs.mStateBits;
}

bool ModifierState::IsPressShift() const { return (mStateBits & SHIFT) != 0; }
void ModifierState::SetPressShift(bool isPressShift) { mStateBits |= (isPressShift ? SHIFT : 0); }

bool ModifierState::IsPressCtrl() const { return (mStateBits & CONTROL) != 0; }
void ModifierState::SetPressCtrl(bool isPressCtrl) { mStateBits |= (isPressCtrl ? CONTROL : 0); }

bool ModifierState::IsPressAlt() const { return (mStateBits & ALT) != 0; }
void ModifierState::SetPressAlt(bool isPressAlt) { mStateBits |= (isPressAlt ? ALT : 0); }

bool ModifierState::IsPressWin() const { return (mStateBits & WIN) != 0; }
void ModifierState::SetPressWin(bool isPressWin) { mStateBits |= (isPressWin ? WIN : 0); }

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
	ret += f(IsPressShift(), _T("Shift"));
	ret += f(IsPressCtrl(), _T("Ctrl"));
	ret += f(IsPressAlt(), _T("Alt"));
	ret += f(IsPressWin(), _T("Win"));
	ret += _T("Enter");
	return ret;
}

} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp


