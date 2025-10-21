#pragma once

namespace launcherapp {
namespace commands {
namespace keysplitter {

enum MODIFIER_MASK {
	SHIFT = 0x1,
	CONTROL = 0x2,
	ALT = 0x4,
	WIN = 0x8,
};

class ModifierState
{
public:
	ModifierState();
	ModifierState(int stateBits);
	ModifierState(bool isPressShift, bool isPressCtrl, bool isPressAlt, bool isPressWin);
	ModifierState(const ModifierState& rhs);

	ModifierState& operator = (const ModifierState&) = default;
	bool operator == (const ModifierState& rhs) const;
	bool operator != (const ModifierState& rhs) const;
	bool operator < (const ModifierState& rhs) const;

	bool IsPressShift() const;
	void SetPressShift(bool isPressShift);
	bool IsPressCtrl() const;
	void SetPressCtrl(bool isPressCtrl);
	bool IsPressAlt() const;
	void SetPressAlt(bool isPressAlt);
	bool IsPressWin() const;
	void SetPressWin(bool isPressWin);

	CString ToString() const;
	int ToBits() const;

	bool mIsPressShift{false};
	bool mIsPressCtrl{false};
	bool mIsPressAlt{false};
	bool mIsPressWin{false};
};


} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp


