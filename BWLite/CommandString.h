#pragma once

#include <vector>

class CommandString
{
public:
	CommandString(const CString& str);
	~CommandString();

public:
	const CString& GetCommandString();
	const CString& GetParameterString();

	void GetParameters(std::vector<CString>& args);

	void AppendParameterPart(CString& str);

	// 補完
	bool ComplementCommand(const CString& commandName, CString& comlementedStr);

protected:
	CString mWholeText;
	CString mCommandPart;
	CString mParamPart;
	bool mHasSpace;

};

