#include "pch.h"
#include "VirtualKeyDefine.h"

VirtualKeyDefine::ITEM::ITEM(UINT vk, LPCTSTR chr, int kind) : 
	mVKCode(vk), mChar(chr), mKind(kind)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static const VirtualKeyDefine::ITEM VK_DEFINED_DATA[] = {
	{ 0x41, _T("A"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x42, _T("B"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x43, _T("C"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x44, _T("D"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x45, _T("E"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x46, _T("F"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x47, _T("G"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x48, _T("H"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x49, _T("I"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x4A, _T("J"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x4B, _T("K"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x4C, _T("L"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x4D, _T("M"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x4E, _T("N"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x4F, _T("O"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x50, _T("P"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x51, _T("Q"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x52, _T("R"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x53, _T("S"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x54, _T("T"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x55, _T("U"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x56, _T("V"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x57, _T("W"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x58, _T("X"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x59, _T("Y"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x5A, _T("Z"), VirtualKeyDefine::KIND_ALPHA },
	{ 0x31, _T("1"), VirtualKeyDefine::KIND_NUMBER },
	{ 0x32, _T("2"), VirtualKeyDefine::KIND_NUMBER },
	{ 0x33, _T("3"), VirtualKeyDefine::KIND_NUMBER },
	{ 0x34, _T("4"), VirtualKeyDefine::KIND_NUMBER },
	{ 0x35, _T("5"), VirtualKeyDefine::KIND_NUMBER },
	{ 0x36, _T("6"), VirtualKeyDefine::KIND_NUMBER },
	{ 0x37, _T("7"), VirtualKeyDefine::KIND_NUMBER },
	{ 0x38, _T("8"), VirtualKeyDefine::KIND_NUMBER },
	{ 0x39, _T("9"), VirtualKeyDefine::KIND_NUMBER },
	{ 0x30, _T("0"), VirtualKeyDefine::KIND_NUMBER },
	{ 0x20, _T("Space"), VirtualKeyDefine::KIND_CHAR },
	{ 0x0D, _T("⏎"), VirtualKeyDefine::KIND_CHAR },
	{ 0x2E, _T("Delete"), VirtualKeyDefine::KIND_CHAR },
	{ 0x09, _T("Tab"), VirtualKeyDefine::KIND_CHAR },
	{ 0x26, _T("↑"), VirtualKeyDefine::KIND_MOVE },
	{ 0x28, _T("↓"), VirtualKeyDefine::KIND_MOVE },
	{ 0x25, _T("←"), VirtualKeyDefine::KIND_MOVE },
	{ 0x27, _T("→"), VirtualKeyDefine::KIND_MOVE },
	{ 0xBC, _T(","), VirtualKeyDefine::KIND_CHAR },
	{ 0xBE, _T("."), VirtualKeyDefine::KIND_CHAR },
	{ 0xBF, _T("/"), VirtualKeyDefine::KIND_CHAR },
	{ 0xBA, _T(":"), VirtualKeyDefine::KIND_CHAR },
	{ 0xBB, _T(";"), VirtualKeyDefine::KIND_CHAR },
	{ 0xC0, _T("@"), VirtualKeyDefine::KIND_CHAR },
	{ 0xDB, _T("["), VirtualKeyDefine::KIND_CHAR },
	{ 0xDD, _T("]"), VirtualKeyDefine::KIND_CHAR },
	{ 0xDE, _T("^"), VirtualKeyDefine::KIND_CHAR },
	{ 0xBD, _T("-"), VirtualKeyDefine::KIND_CHAR },
	{ 0x22, _T("PageDown"), VirtualKeyDefine::KIND_MOVE },
	{ 0x21, _T("PageUp"), VirtualKeyDefine::KIND_MOVE },
	{ 0x24, _T("Home"), VirtualKeyDefine::KIND_MOVE },
	{ 0x23, _T("End"), VirtualKeyDefine::KIND_MOVE },
	{ 0x2D, _T("Insert"), VirtualKeyDefine::KIND_MOVE },
	{ 0x61, _T("Num 1"), VirtualKeyDefine::KIND_NUMKEY },
	{ 0x62, _T("Num 2"), VirtualKeyDefine::KIND_NUMKEY },
	{ 0x63, _T("Num 3"), VirtualKeyDefine::KIND_NUMKEY },
	{ 0x64, _T("Num 4"), VirtualKeyDefine::KIND_NUMKEY },
	{ 0x65, _T("Num 5"), VirtualKeyDefine::KIND_NUMKEY },
	{ 0x66, _T("Num 6"), VirtualKeyDefine::KIND_NUMKEY },
	{ 0x67, _T("Num 7"), VirtualKeyDefine::KIND_NUMKEY },
	{ 0x68, _T("Num 8"), VirtualKeyDefine::KIND_NUMKEY },
	{ 0x69, _T("Num 9"), VirtualKeyDefine::KIND_NUMKEY },
	{ 0x60, _T("Num 0"), VirtualKeyDefine::KIND_NUMKEY },
	{ 0xF0, _T("CapsLock"), VirtualKeyDefine::KIND_OTHER },
	{ 0xF2, _T("かな"), VirtualKeyDefine::KIND_OTHER },
	{ 0x1C, _T("変換"), VirtualKeyDefine::KIND_OTHER },
	{ 0x1D, _T("無変換"), VirtualKeyDefine::KIND_OTHER },
	{ 0x90, _T("NumLock"), VirtualKeyDefine::KIND_OTHER },
	{ 0x70, _T("F1"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x71, _T("F2"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x72, _T("F3"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x73, _T("F4"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x74, _T("F5"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x75, _T("F6"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x76, _T("F7"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x77, _T("F8"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x78, _T("F9"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x79, _T("F10"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x7A, _T("F11"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x7B, _T("F12"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x7C, _T("F13"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x7D, _T("F14"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x7E, _T("F15"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x7F, _T("F16"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x80, _T("F17"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x81, _T("F18"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x82, _T("F19"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x83, _T("F20"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x84, _T("F21"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x85, _T("F22"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x86, _T("F23"), VirtualKeyDefine::KIND_FUNCTION },
	{ 0x87, _T("F24"), VirtualKeyDefine::KIND_FUNCTION },
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

VirtualKeyDefine::VirtualKeyDefine()
{
}

VirtualKeyDefine::~VirtualKeyDefine()
{
}

int VirtualKeyDefine::GetItemCount() const
{
	return sizeof(VK_DEFINED_DATA) / sizeof(VK_DEFINED_DATA[0]);
}

VirtualKeyDefine::ITEM VirtualKeyDefine::GetItem(int index) const
{
	return VK_DEFINED_DATA[index];
}

VirtualKeyDefine* VirtualKeyDefine::GetInstance()
{
	static VirtualKeyDefine inst;
	return &inst;	
}
