#include "stdafx.h"
#include "gtest/gtest.h"
#include "icon/IconLoader.h"

class IconLoaderTest : public ::testing::Test {
protected:
    IconLoader* loader;
    void SetUp() override {
        loader = IconLoader::Get();
    }
    void TearDown() override {
        // 破棄はIconLoaderのシングルトンに任せる
    }
};

TEST_F(IconLoaderTest, GetInstanceReturnsSingleton) {
    IconLoader* loader2 = IconLoader::Get();
    EXPECT_EQ(loader, loader2);
}

TEST_F(IconLoaderTest, LoadUnknownIconReturnsValidHandle) {
    HICON icon = loader->LoadUnknownIcon();
    EXPECT_NE(icon, nullptr);
    EXPECT_TRUE(loader->HasIcon(icon));
}

TEST_F(IconLoaderTest, LoadDefaultIconReturnsValidHandle) {
    HICON icon = loader->LoadDefaultIcon();
    EXPECT_NE(icon, nullptr);
    EXPECT_TRUE(loader->HasIcon(icon));
}

TEST_F(IconLoaderTest, LoadIconFromPathWithInvalidPathReturnsUnknownIcon) {
    CString invalidPath(_T("Z:\\notfound.file"));
    HICON icon = loader->LoadIconFromPath(invalidPath);
    EXPECT_NE(icon, nullptr);
    EXPECT_TRUE(loader->HasIcon(icon));
}

TEST_F(IconLoaderTest, LoadIconFromImageFileWithInvalidPathReturnsNull) {
    CString invalidPath(_T("Z:\\notfound.png"));
    HICON icon = loader->LoadIconFromImageFile(invalidPath, false);
    EXPECT_EQ(icon, nullptr);
}

TEST_F(IconLoaderTest, LoadIconResourceWithInvalidDllReturnsNull) {
    CString invalidDll(_T("Z:\\notfound.dll"));
    HICON icon = loader->LoadIconResource(invalidDll, 0);
    EXPECT_EQ(icon, nullptr);
}

TEST_F(IconLoaderTest, GetStreamFromPathWithInvalidPathReturnsFalse) {
    std::vector<uint8_t> strm;
    CString invalidPath(_T("Z:\\notfound.png"));
    bool result = IconLoader::GetStreamFromPath(invalidPath, strm);
    EXPECT_FALSE(result);
}

TEST_F(IconLoaderTest, TryGetStreamFromIconPathWithInvalidPathReturnsFalse) {
    std::vector<uint8_t> strm;
    CString invalidPath(_T("Z:\\notfound.ico"));
    bool result = IconLoader::TryGetStreamFromIconPath(invalidPath, strm);
    EXPECT_FALSE(result);
}

// 追加で他のpublicメソッドも同様にテストできます
// 例: LoadFolderIcon, LoadWebIcon, LoadEditIcon, LoadVolumeIcon など

TEST_F(IconLoaderTest, LoadExtensionIconWithInvalidExtensionReturnsNullOrValidIcon) {
    CString ext(_T(".notexistext"));
    HICON icon = loader->LoadExtensionIcon(ext);
    // 取得できない場合はnullptr、取得できる場合はHasIconで管理されている
    // どちらでもOK
    if (icon != nullptr) {
        EXPECT_TRUE(loader->HasIcon(icon));
    }
}

TEST_F(IconLoaderTest, LoadIconFromHwndWithInvalidHwndReturnsWindowIcon) {
    HWND hwnd = (HWND)0x123456; // 存在しないハンドル
    HICON icon = loader->LoadIconFromHwnd(hwnd);
    EXPECT_NE(icon, nullptr);
    EXPECT_TRUE(loader->HasIcon(icon));
}

TEST_F(IconLoaderTest, LoadIconFromHwndWithCurrentProcessWindowReturnsWindowIcon) {
    HWND hwnd = GetConsoleWindow(); // 自プロセスのウィンドウ
    HICON icon = loader->LoadIconFromHwnd(hwnd);
    EXPECT_NE(icon, nullptr);
    EXPECT_TRUE(loader->HasIcon(icon));
}

TEST_F(IconLoaderTest, LoadFolderIconReturnsValidIcon) {
    HICON icon = loader->LoadFolderIcon();
    EXPECT_NE(icon, nullptr);
    EXPECT_TRUE(loader->HasIcon(icon));
}

TEST_F(IconLoaderTest, LoadWebIconReturnsValidIcon) {
    HICON icon = loader->LoadWebIcon();
    EXPECT_NE(icon, nullptr);
    EXPECT_TRUE(loader->HasIcon(icon));
}

TEST_F(IconLoaderTest, LoadEditIconReturnsValidIcon) {
    HICON icon = loader->LoadEditIcon();
    EXPECT_NE(icon, nullptr);
    EXPECT_TRUE(loader->HasIcon(icon));
}

TEST_F(IconLoaderTest, LoadVolumeIconReturnsValidIcon) {
    HICON iconMute = loader->LoadVolumeIcon(true);
    HICON iconNormal = loader->LoadVolumeIcon(false);
    EXPECT_NE(iconMute, nullptr);
    EXPECT_NE(iconNormal, nullptr);
    EXPECT_TRUE(loader->HasIcon(iconMute));
    EXPECT_TRUE(loader->HasIcon(iconNormal));
}

TEST_F(IconLoaderTest, LoadIconFromStreamWithInvalidDataReturnsNull) {
    std::vector<uint8_t> dummy(10, 0xFF);
    HICON icon = loader->LoadIconFromStream(dummy);
    // 失敗時はnullptr
    EXPECT_EQ(icon, nullptr);
}

TEST_F(IconLoaderTest, LoadRegisterWindowIconReturnsValidIcon) {
    HICON icon = loader->LoadRegisterWindowIcon();
    EXPECT_NE(icon, nullptr);
    EXPECT_TRUE(loader->HasIcon(icon));
}

TEST_F(IconLoaderTest, LoadGroupIconReturnsValidIcon) {
    HICON icon = loader->LoadGroupIcon();
    EXPECT_NE(icon, nullptr);
    EXPECT_TRUE(loader->HasIcon(icon));
}

TEST_F(IconLoaderTest, LoadPromptIconReturnsValidIcon) {
    HICON icon = loader->LoadPromptIcon();
    EXPECT_NE(icon, nullptr);
    EXPECT_TRUE(loader->HasIcon(icon));
}

TEST_F(IconLoaderTest, LoadConvertIconReturnsValidIcon) {
    HICON icon = loader->LoadConvertIcon();
    EXPECT_NE(icon, nullptr);
    EXPECT_TRUE(loader->HasIcon(icon));
}

TEST_F(IconLoaderTest, LoadHistoryIconReturnsValidIcon) {
    HICON icon = loader->LoadHistoryIcon();
    EXPECT_NE(icon, nullptr);
    EXPECT_TRUE(loader->HasIcon(icon));
}


