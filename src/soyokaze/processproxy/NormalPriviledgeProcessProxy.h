#pragma once

#include <cstdint>
#include <memory>
#include <map>
#include <string>
#include <vector>

#pragma warning( push )
#pragma warning( disable : 26800 26819 )
#include <nlohmann/json.hpp>
#pragma warning( pop )

namespace launcherapp { namespace processproxy {

namespace detail {

enum class NormalPriviledgeAgentStatus {
	Available,
	Busy,
	NotResponding
};

enum class NormalPriviledgeAgentAction {
	Start,
	Reuse,
	Wait
};

/**
  通常権限エージェントの接続状態と、状態に応じた次の処理を管理する
*/
class NormalPriviledgeAgentConnectionState
{
public:
	/**
	  ウィンドウの有無とエージェントの応答状態から次の処理を決定する
	  @param[in] hasServerWindow サーバーウィンドウが存在するか
	  @param[in] status サーバーの応答状態
	  @return 次に実行する処理
	*/
	NormalPriviledgeAgentAction Evaluate(bool hasServerWindow, NormalPriviledgeAgentStatus status)
	{
		if (mIsStarted == false) {
			return NormalPriviledgeAgentAction::Start;
		}
		if (hasServerWindow == false || status == NormalPriviledgeAgentStatus::NotResponding) {
			mIsStarted = false;
			return NormalPriviledgeAgentAction::Start;
		}
		if (status == NormalPriviledgeAgentStatus::Busy) {
			return NormalPriviledgeAgentAction::Wait;
		}
		return NormalPriviledgeAgentAction::Reuse;
	}

	/**
	  エージェントの起動が確認できた状態にする
	*/
	void MarkStarted()
	{
		mIsStarted = true;
	}

	/**
	  エージェントの接続状態を未起動に戻す
	*/
	void Reset()
	{
		mIsStarted = false;
	}

	/**
	  エージェントが起動済みかどうかを取得する
	  @return 起動済みの場合true
	*/
	bool IsStarted() const
	{
		return mIsStarted;
	}

private:
	bool mIsStarted{false};
};

} // namespace detail

class NormalPriviledgeProcessProxy
{
	NormalPriviledgeProcessProxy();
	~NormalPriviledgeProcessProxy();

public:
	static NormalPriviledgeProcessProxy* GetInstance();

	bool SendRequest(nlohmann::json& req, nlohmann::json& res);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}  // end of namespace launcherapp::processproxy

