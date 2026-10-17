#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/WinHttp.h"
#include "SharedHwnd.h"
#include <filesystem>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>


using json = nlohmann::json;
using namespace launcherapp;

namespace {

  std::wstring GetApplicationPath()
  {
	wchar_t path[MAX_PATH];

	GetModuleFileNameW(
		nullptr,
		path,
		_countof(path));

	std::filesystem::path exePath(path);

	auto appPath =
	  exePath.parent_path() /
	  L"..\\Release\\soyokaze.exe";

	return std::filesystem::weakly_canonical(appPath);
  }

}

TEST(WinAppDriverTest, LaunchNotepad)
{
	SharedHwnd sharedHwnd;

  auto appPath = GetApplicationPath();
  ASSERT_TRUE(PathFileExists(appPath.c_str()));

	auto hwndHex = std::format("0x{:X}", reinterpret_cast<uintptr_t>(sharedHwnd.GetHwnd()));
  // セッション作成要求
	json request = {
		{
		"desiredCapabilities", {
			{"platformName", "Windows"},
			{"deviceName", "WindowsPC"},
			{"appTopLevelWindow", hwndHex }
		}
		}
	};

  WinHttp http;

  http.SetMethod(L"POST");
  http.SetContentType(L"application/json; charset=utf-8");

  auto req_data = request.dump();

  std::string body = request.dump(2);
    std::cout << body << std::endl;

  HttpResponse response;
  ASSERT_TRUE(http.Request(L"http://127.0.0.1:4723/session", req_data, response));

	std::cout << response.body << std::endl;

  EXPECT_EQ(response.statusCode, 200);

  json result = json::parse(response.body);

//  body = response.dump();
//  std::cout << body << std::endl;
   

  ASSERT_TRUE(result.contains("sessionId"));

  auto sessionId = result["sessionId"]	.get<std::string>();

  EXPECT_FALSE(sessionId.empty());
}
