#include "EnumPresentationSlidesProxyCommand.h"
#include <windows.h>
#include "processproxy/share/AfxWFunctions.h"
#include "StringUtil.h"
#include "AutoWrap.h"
#include <oleauto.h>
#include <atlbase.h>
#include <regex>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace launcherproxy { 

REGISTER_PROXYCOMMAND(EnumPresentationSlidesProxyCommand)

EnumPresentationSlidesProxyCommand::EnumPresentationSlidesProxyCommand()
{
}

EnumPresentationSlidesProxyCommand::~EnumPresentationSlidesProxyCommand()
{
}

std::string EnumPresentationSlidesProxyCommand::GetName()
{
	return "enumpresentationslides";
}


static std::wstring GetSlideTitle(DispWrapper& slide)
{
	// Shapes
	DispWrapper shapes;
	slide.GetPropertyObject(L"Shapes", shapes);

	// Shapes.Count
	int shapeCount = shapes.GetPropertyInt(L"Count");

	// foreach(shapes)
	for (int16_t j = 1; j <= (int16_t)shapeCount; ++j) {

		DispWrapper shape;
		shapes.CallObjectMethod(L"Item", j, shape);

		// Type
		int type = shape.GetPropertyInt(L"Type");
		if (type != 14 && type != 17) {  // 14:PlaceHolder 17:TextBox
			continue;
		}

		// Shape.Name
		std::wstring shapeName = shape.GetPropertyString(L"Name");

		// shape名がTitleでなければスキップ
		static std::wregex pat(_T("^ *Title.*$"));      // 英語版向け
		static std::wregex pat2(_T("^ *タイトル.*$"));  // 日本語版向け
		if (std::regex_match(shapeName, pat) == false && 
				std::regex_match(shapeName, pat2) == false) {
			continue;
		}

		// スライドタイトルを取得(shape.TextFrame2.TextRange.Item(1))
		DispWrapper txtFrame;
		shape.GetPropertyObject(L"TextFrame2", txtFrame);
		DispWrapper txtRange;
		txtFrame.GetPropertyObject(L"TextRange", txtRange);

		return txtRange.GetPropertyString(L"Text");
	}
	return _T("");
}

bool EnumPresentationSlidesProxyCommand::Execute(json& json_req, json& json_res)
{
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"PowerPoint.Application", &clsid);
	if (FAILED(hr)) {
		// インストールされていない
		json_res["result"] = false;
		json_res["reason"] = "PowerPoint is not installed.";
		return true;
	}

	// アプリケーションを取得
	CComPtr<IUnknown> unkPtr;
	hr = GetActiveObject(clsid, NULL, &unkPtr);
	if (FAILED(hr)) {
		// 起動してない
		json_res["result"] = false;
		json_res["reason"] = "PowerPoint is not running.";
		return true;
	}

	DispWrapper pptApp;
	unkPtr->QueryInterface(&pptApp);

	// アクティブなPresentationを取得
	DispWrapper activePresentation;
	if (pptApp.GetPropertyObject(L"ActivePresentation", activePresentation) == false) {
		// アプリは起動してるけど何も表示してない(初期画面とか)
		json_res["result"] = false;
		json_res["reason"] = "Presentaion is not opened.";
		return true;
	}

	// ファイルパスを取得
	std::wstring fileDir = activePresentation.GetPropertyString(L"Path");
	std::wstring fileName = activePresentation.GetPropertyString(L"Name");

	std::string tmp;

	json_res["file_path"] = utf2utf(fileDir + _T("\\") + fileName, tmp);

	// Slidesを取得
	DispWrapper slides;
	activePresentation.GetPropertyObject(L"Slides", slides);

	// Count
	int slideCount = slides.GetPropertyInt(L"Count");

	std::vector<std::map<std::string, std::string>> values;

	values.reserve(slideCount);
	for (int16_t i = 1; i <= (int16_t)slideCount; ++i) {

		// スライドを取得
		DispWrapper slide;
		slides.CallObjectMethod(L"Item", i, slide);


		// 取得したタイトル,indexを登録
		values.emplace_back(std::map<std::string, std::string>{{"title", utf2utf(GetSlideTitle(slide), tmp)}});
	}

	json_res["result"] = true;
	json_res["slides"] = values;

	return true;
}

} // end of namespace 



