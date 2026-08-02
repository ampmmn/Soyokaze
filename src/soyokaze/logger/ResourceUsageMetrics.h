#pragma once

#include "IResourceMetrics.h"

namespace logger {

// プロセスの現在のワーキングセットサイズを取得するメトリクス。
class WorkingSetMetrics : public IResourceMetrics
{
public:
	WorkingSetMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

// プロセスの現在のプライベートバイト数を取得するメトリクス。
class PrivateBytesMetrics : public IResourceMetrics
{
public:
	PrivateBytesMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

// プロセスのスレッド数を取得するメトリクス。
class ThreadMetrics : public IResourceMetrics
{
public:
	ThreadMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

// GDIオブジェクト数を取得するメトリクス。
class GdiObjectsMetrics : public IResourceMetrics
{
public:
	GdiObjectsMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

// USERオブジェクト数を取得するメトリクス。
class UserObjectsMetrics : public IResourceMetrics
{
public:
	UserObjectsMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
