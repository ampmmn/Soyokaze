#pragma once

#include <string>

namespace logger {

class IResourceMetrics
{
public:
	virtual ~IResourceMetrics() = default;

	// CSVの列順を決めるための順序を取得する。
	virtual int GetOrder() const = 0;
	// CSVヘッダーに出力するメトリクス名を取得する。
	virtual std::string GetName() const = 0;
	// CSVのデータ行に出力するメトリクス値を取得する。
	virtual std::string GetValue() const = 0;
};

}
