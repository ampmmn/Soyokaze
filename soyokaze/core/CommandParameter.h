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
	const CString& GetCommandString() const;
	const CString& GetParameterString() const;

	void GetParameters(std::vector<CString>& args) const;

	bool GetExtraBoolParam(LPCTSTR name) const;
	void SetExtraBoolParam(LPCTSTR name, bool value);

	void AppendParameterPart(CString& str);

	// 補完
	bool ComplementCommand(const CString& commandName, CString& comlementedStr) const;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}

