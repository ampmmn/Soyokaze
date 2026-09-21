#pragma once

struct PythonProxyIF
{
	virtual bool CompileTest(const char* src, char** errMsg) = 0;
	virtual bool Evaluate(const char* src, const char** argv, char** errMsg) = 0;
	virtual bool EvalForCalculate(const char* src, char** result) = 0;
	/**
	  計算用のPythonスクリプトを実行し、結果変数の値を取得する
	  @return true:成功 false:失敗
	  @param[in] src 実行するPythonスクリプト
	  @param[out] result スクリプト内の__soyokaze_resultの値
	*/
	virtual bool EvalScriptForCalculate(const char* src, char** result) = 0;
	virtual void ReleaseBuffer(char* result) = 0;
	virtual bool IsPyCmdAvailable() = 0;
	virtual bool IsBusy() = 0;
};


