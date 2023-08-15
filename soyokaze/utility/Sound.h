#pragma once

namespace utility {

// 効果音を再生するためのクラス
class Sound
{
public:
	struct ITEM;
private:
	Sound();
	~Sound();

public:
	static Sound* Get();

	// 非同期で音声ファイルを再生する
	bool PlayAsync(LPCTSTR filePath);

private:
	static LRESULT CALLBACK OnPlayCallbackProc(HWND h, UINT msg, WPARAM wp, LPARAM lp);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of utility

