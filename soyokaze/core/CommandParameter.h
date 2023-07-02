#pragma once

#include <vector>
#include <memory>

namespace soyokaze {
namespace core {

class CommandParameter
{
public:
	CommandParameter();
	CommandParameter(const CString& str);
	~CommandParameter();

public:
	void AddArgument(const CString& arg);

	void SetWholeString(const CString& str);
	const CString& GetWholeString() const;
	const CString& GetCommandString() const;
	const CString& GetParameterString() const;


	void CopyParamTo(CommandParameter& rhs) const;

	void GetParameters(std::vector<CString>& args) const;
	static void GetParameters(const CString& paramStr, std::vector<CString>& args);

	bool GetNamedParam(LPCTSTR name, CString* value) const;

	CString GetNamedParamString(LPCTSTR name) const;
	void SetNamedParamString(LPCTSTR name, LPCTSTR value);

	bool GetNamedParamBool(LPCTSTR name) const;
	void SetNamedParamBool(LPCTSTR name, bool value);

	void AppendParameterPart(CString& str);

	// 補完
	bool ComplementCommand(const CString& commandName, CString& comlementedStr) const;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}

