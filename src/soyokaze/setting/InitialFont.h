#pragma once

class InitialFont
{
public:
	InitialFont();
	~InitialFont();

public:
	void GetFontName(CString& name);
	int GetFontSize();

private:
	CString mFontName;
	int mFontSize;

};

