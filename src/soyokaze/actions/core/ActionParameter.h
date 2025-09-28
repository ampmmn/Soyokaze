#pragma once

#include "core/UnknownIF.h"
#include "actions/core/ActionParameterIF.h"

#include <vector>
#include <memory>

namespace launcherapp { namespace actions { namespace core {

class ParameterBuilder : 
	virtual public Parameter,
	virtual public NamedParameter
{
private:
	ParameterBuilder();
	ParameterBuilder(const ParameterBuilder& rhs);
	ParameterBuilder(const CString& str);
	~ParameterBuilder();

	ParameterBuilder& operator = (const ParameterBuilder& rhs);

public:
	static ParameterBuilder* Create();
	static ParameterBuilder* Create(const CString& str);
	ParameterBuilder* Clone_() const;

	// EmptyParamは毎回同じインスタンスを返す。変更したり削除しないこと
	static ParameterBuilder* EmptyParam();

public:
	void AddArgument(const CString& arg);
	void CopyParamTo(ParameterBuilder& rhs) const;
	void CopyNamedParamTo(ParameterBuilder& rhs) const;

	// 補完
	bool ComplementCommand(const CString& commandName, CString& comlementedStr) const;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;
	uint32_t AddRef() override;
	uint32_t Release() override;

// Parameter
	bool IsEmpty() const override;
	bool HasParameter() const override;
	LPCTSTR GetWholeString() const override;
	LPCTSTR GetCommandString() const override;
	LPCTSTR GetParameterString() const override;
	int GetParamCount() const override;
	LPCTSTR GetParam(int index) const override;
	void SetWholeString(LPCTSTR param) override;
	void SetParameterString(LPCTSTR param) override;
	Parameter* Clone() const override;

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

}}}

