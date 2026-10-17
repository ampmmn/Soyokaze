#include "stdafx.h"
#include "gtest/gtest.h"
#include "icon/IconLoader.h"

#include <cstdint>
#include <limits>
#include <objbase.h>
#include <shobjidl.h>
#include <vector>
#include <wrl/client.h>

namespace
{
    class ComApartment
    {
    public:
        ComApartment()
            : mResult(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))
        {
        }

        ~ComApartment()
        {
            if (SUCCEEDED(mResult)) {
                CoUninitialize();
            }
        }

        bool IsAvailable() const
        {
            return SUCCEEDED(mResult) || mResult == RPC_E_CHANGED_MODE;
        }

    private:
        HRESULT mResult;
    };

    /**
      テスト用に指定アイコン場所を持つShell Linkデータを作成する
      @param[in] iconPath アイコンファイルのパス
      @param[in] iconIndex アイコンのインデックス
      @param[out] linkData 作成したShell Linkデータ
      @return true:成功 false:失敗
    */
    bool CreateShellLinkData(const CString& iconPath, int iconIndex, std::vector<uint8_t>& linkData)
    {
        Microsoft::WRL::ComPtr<IShellLinkW> shellLink;
        if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(shellLink.GetAddressOf())))) {
            return false;
        }
        if (FAILED(shellLink->SetPath(L"C:\\test.exe")) ||
            FAILED(shellLink->SetIconLocation(iconPath, iconIndex))) {
            return false;
        }

        Microsoft::WRL::ComPtr<IPersistStream> persistStream;
        if (FAILED(shellLink.As(&persistStream))) {
            return false;
        }

        Microsoft::WRL::ComPtr<IStream> stream;
        if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, stream.GetAddressOf())) ||
            FAILED(persistStream->Save(stream.Get(), TRUE))) {
            return false;
        }

        STATSTG stat{};
        if (FAILED(stream->Stat(&stat, STATFLAG_NONAME)) ||
            stat.cbSize.QuadPart > (std::numeric_limits<ULONG>::max)()) {
            return false;
        }

        linkData.resize(static_cast<std::size_t>(stat.cbSize.QuadPart));
        LARGE_INTEGER streamStart{};
        if (FAILED(stream->Seek(streamStart, STREAM_SEEK_SET, nullptr))) {
            return false;
        }

        ULONG bytesRead = 0;
        return SUCCEEDED(stream->Read(linkData.data(), static_cast<ULONG>(linkData.size()), &bytesRead)) &&
            bytesRead == linkData.size();
    }
}

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

TEST_F(IconLoaderTest, GetDefaultIconReusesCachedHandle) {
    TCHAR systemDirectory[MAX_PATH] = {};
    UINT length = GetSystemDirectory(systemDirectory, MAX_PATH);
    ASSERT_GT(length, 0u);
    ASSERT_LT(length, static_cast<UINT>(MAX_PATH));

    CString iconPath(systemDirectory);
    iconPath += _T("\\shell32.dll,0");

    auto statisticsBefore = loader->GetCacheStatistics();
    HICON firstIcon = loader->GetDefaultIcon(iconPath);
    ASSERT_NE(firstIcon, nullptr);
    auto statisticsAfterFirst = loader->GetCacheStatistics();
    HICON secondIcon = loader->GetDefaultIcon(iconPath);
    auto statisticsAfterSecond = loader->GetCacheStatistics();

    EXPECT_EQ(firstIcon, secondIcon);
    EXPECT_EQ(statisticsBefore.defaultIconCacheCount + 1, statisticsAfterFirst.defaultIconCacheCount);
    EXPECT_EQ(statisticsAfterFirst.defaultIconCacheCount, statisticsAfterSecond.defaultIconCacheCount);
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

TEST_F(IconLoaderTest, LoadLnkIconReturnsCachedLargeIcon)
{
    ComApartment comApartment;
    ASSERT_TRUE(comApartment.IsAvailable());

    TCHAR systemDirectory[MAX_PATH] = {};
    UINT length = GetSystemDirectory(systemDirectory, MAX_PATH);
    ASSERT_GT(length, 0u);
    ASSERT_LT(length, static_cast<UINT>(MAX_PATH));

    CString iconPath(systemDirectory);
    iconPath += _T("\\shell32.dll");

    std::vector<uint8_t> linkData;
    ASSERT_TRUE(CreateShellLinkData(iconPath, 0, linkData));

    HICON firstIcon = loader->LoadLnkIcon(linkData);
    ASSERT_NE(firstIcon, nullptr);
    EXPECT_TRUE(loader->HasIcon(firstIcon));

    auto statisticsAfterFirst = loader->GetCacheStatistics();
    HICON secondIcon = loader->LoadLnkIcon(linkData);
    auto statisticsAfterSecond = loader->GetCacheStatistics();

    EXPECT_EQ(firstIcon, secondIcon);
    EXPECT_EQ(statisticsAfterFirst.iconIndexCacheIconCount, statisticsAfterSecond.iconIndexCacheIconCount);
}

TEST_F(IconLoaderTest, LoadLnkIconReturnsNullForInvalidData)
{
    std::vector<uint8_t> invalidData;
    EXPECT_EQ(nullptr, loader->LoadLnkIcon(invalidData));
}

TEST_F(IconLoaderTest, GetStreamFromPathWithInvalidPathReturnsFalse) {
    std::vector<uint8_t> strm;
    CString invalidPath(_T("Z:\\notfound.png"));
    bool result = IconLoader::GetStreamFromPath(invalidPath, strm);
    EXPECT_FALSE(result);
}

TEST_F(IconLoaderTest, GetStreamFromPathWithInvalidUrlReturnsFalse) {
    std::vector<uint8_t> strm;
    CString invalidUrl(_T("http://"));
    bool result = IconLoader::GetStreamFromPath(invalidUrl, strm);
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


