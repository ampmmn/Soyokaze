#pragma once

#include <memory>

namespace launcherapp {
namespace mainwindow {

// 連続稼働時間を警告するトースト
class Toast
{
public:
	Toast();
	~Toast();

	void SetThreshold(int th);

	// トーストを表示する
	void Show();
	// このクラスで表示した既存のトーストを消す
	void Clear();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

}
}

