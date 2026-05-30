#pragma once


class WipingString : public CString
{
public:
	WipingString();
	WipingString(LPCSTR s);
	~WipingString();

	WipingString& operator = (const CString& str);

};

