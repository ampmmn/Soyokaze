#pragma once

#include <vector>
#include <memory>

namespace launcherapp {
namespace core {

class CommandParameter
{
public:
	CommandParameter();
	CommandParameter(const CommandParameter& rhs);
	CommandParameter(const CString& str);
	~CommandParameter();

	CommandParameter& operator = (const CommandParameter& rhs);

public:
	bool IsEmpty() const;
	bool HasParameter() const;

	void AddArgument(const CString& arg);

	void SetWholeString(const CString& str);
	void SetParamString(const CString& paramStr);
	const CString& GetWholeString() const;
	const CString& GetCommandString() const;
	const CString& GetParameterString() const;


	void CopyParamTo(CommandParameter& rhs) const;
	void CopyNamedParamTo(CommandParameter& rhs) const;

	void GetParameters(std::vector<CString>& args) const;
	static void GetParameters(const CString& paramStr, std::vector<CString>& args);

	bool GetNamedParam(LPCTSTR name, CString* value) const;

	CString GetNamedParamString(LPCTSTR name) const;
	void SetNamedParamString(LPCTSTR name, LPCTSTR value);

	bool GetNamedParamBool(LPCTSTR name) const;
	void SetNamedParamBool(LPCTSTR name, bool value);

	// 補完
	bool ComplementCommand(const CString& commandName, CString& comlementedStr) const;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}

