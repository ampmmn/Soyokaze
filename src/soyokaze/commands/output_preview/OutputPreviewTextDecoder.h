#pragma once

namespace launcherapp {
namespace commands {
namespace output_preview {

class OutputPreviewTextDecoder
{
public:
	/**
	  UTF-8として有効ならUTF-8、そうでなければSJISとして文字列に変換する
	  @param[in] source 変換するヌル終端文字列
	  @param[out] text 変換結果
	*/
	static void Decode(const char* source, CString& text);
};

}
}
}
