#pragma once

#include "commands/core/CommandParameterIF.h"

#include <vector>
#include <memory>

namespace launcherapp {
namespace core {

class CommandParameterBuilder : 
	virtual public CommandParameter,
	virtual public CommandNamedParameter
{
private:
	CommandParameterBuilder();
	CommandParameterBuilder(const CommandParameterBuilder& rhs);
	CommandParameterBuilder(const CString& str);
	~CommandParameterBuilder();

	CommandParameterBuilder& operator = (const CommandParameterBuilder& rhs);

public:
	static CommandParameterBuilder* Create();
	static CommandParameterBuilder* Create(const CString& str);
	CommandParameterBuilder* Clone_() const;

	// EmptyParamは毎回同じインスタンスを返す。変更したり削除しないこと
	static CommandParameterBuilder* EmptyParam();

public:
	void AddArgument(const CString& arg);
	void CopyParamTo(CommandParameterBuilder& rhs) const;
	void CopyNamedParamTo(CommandParameterBuilder& rhs) const;

	// 補完
	bool ComplementCommand(const CString& commandName, CString& comlementedStr) const;

// UnknownIF
	bool QueryInterface(const IFID& ifid, void** cmd) override;
	uint32_t AddRef() override;
	uint32_t Release() override;

// CommandParameter
	bool IsEmpty() const override;
	bool HasParameter() const override;
	LPCTSTR GetWholeString() const override;
	LPCTSTR GetCommandString() const override;
	LPCTSTR GetParameterString() const override;
	int GetParamCount() const override;
	LPCTSTR GetParam(int index) const override;
	void SetWholeString(LPCTSTR param) override;
	void SetParameterString(LPCTSTR param) override;
	CommandParameter* Clone() const override;

// CommandNamedParameter
	int GetNamedParamStringLength(LPCTSTR name) const override;
	LPCTSTR GetNamedParamString(LPCTSTR name, LPTSTR buf, int bufLen) const override;
	void SetNamedParamString(LPCTSTR name, LPCTSTR value) override;
	bool GetNamedParamBool(LPCTSTR name) const override;
	void SetNamedParamBool(LPCTSTR name, bool value) override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}

