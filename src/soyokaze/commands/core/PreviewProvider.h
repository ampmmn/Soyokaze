#pragma once

#include "core/UnknownIF.h"

namespace launcherapp {
namespace mainwindow {
namespace preview {
class PreviewWindow;
}
}

namespace core {

class PreviewProvider : virtual public UnknownIF
{
public:
	virtual ~PreviewProvider() {}

	/**
	  表示するプレビューウインドウを取得する
	  @return プレビューウインドウ。提供しない場合はnullptr
	*/
	virtual launcherapp::mainwindow::preview::PreviewWindow* FetchPreviewWindow() = 0;
};

}
}
