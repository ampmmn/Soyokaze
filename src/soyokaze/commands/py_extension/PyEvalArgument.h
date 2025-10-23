#pragma once

#include "actions/core/ActionParameter.h"
#include <vector>

class PyEvalArgument
{
public:
	PyEvalArgument(const CString& argsStr) 
	{
		auto param = launcherapp::actions::core::ParameterBuilder::Create(_T(""));
		param->SetParameterString(argsStr);

		CString tmpW;
		std::string tmpA;

		int n = param->GetParamCount();
		auto args = new char*[n + 1];

		for (int i = 0; i < n; ++i) {
			tmpW = param->GetParam(i);
			UTF2UTF(tmpW, tmpA);
			auto arg = new char[tmpA.size() + 1];
			memcpy(arg, tmpA.c_str(), tmpA.size() + 1);
			args[i] = arg;
		}
		// 終端としての目印となるnullを追加
		args[n] = nullptr;
		mArgs = args;

		param->Release();
	}

	PyEvalArgument(launcherapp::actions::core::Parameter* param)
	{
		CString tmpW;
		std::string tmpA;

		int n = param->GetParamCount();
		auto args = new char*[n + 1];

		for (int i = 0; i < n; ++i) {
			tmpW = param->GetParam(i);
			UTF2UTF(tmpW, tmpA);
			auto arg = new char[tmpA.size() + 1];
			memcpy(arg, tmpA.c_str(), tmpA.size() + 1);
			args[i] = arg;
		}

		// 終端としての目印となるnullを追加
		args[n] = nullptr;
		mArgs = args;

		param->Release();
	}

	PyEvalArgument(char** args) : mArgs(args)
	{
	}

	~PyEvalArgument()
 	{
		if (mArgs) {
			auto p = mArgs;
			while(p && *p) {
				delete [] *p;
				p++;
			}
		}
	}

	char** get() {
		return mArgs;
	}

	char** release() {
		char** args = mArgs;
		mArgs = nullptr;
		return args;
	}

private:
	char** mArgs{nullptr};
};
