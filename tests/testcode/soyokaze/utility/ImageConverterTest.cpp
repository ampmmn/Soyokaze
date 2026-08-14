#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/ImageConverter.h"

#include <gdiplus.h>

#pragma comment(lib, "Gdiplus.lib")

using namespace Gdiplus;

namespace {

/**
  PNGエンコーダーのCLSIDを取得する
  @return true:成功  false:失敗
  @param[out] clsid PNGエンコーダーのCLSID
*/
bool GetPngEncoderClsid(CLSID& clsid)
{
    UINT encoderCount = 0;
    UINT bufferSize = 0;
    if (Gdiplus::GetImageEncodersSize(&encoderCount, &bufferSize) != Gdiplus::Ok) {
        return false;
    }

    std::vector<BYTE> buffer(bufferSize);
    auto encoders = reinterpret_cast<Gdiplus::ImageCodecInfo*>(buffer.data());
    if (Gdiplus::GetImageEncoders(encoderCount, bufferSize, encoders) != Gdiplus::Ok) {
        return false;
    }

    for (UINT i = 0; i < encoderCount; ++i) {
        if (wcscmp(encoders[i].MimeType, L"image/png") == 0) {
            clsid = encoders[i].Clsid;
            return true;
        }
    }
    return false;
}

/**
  BitmapをPNGデータへ変換する
  @return true:成功  false:失敗
  @param[in] bitmap 変換元のBitmap
  @param[out] output 変換後のPNGデータ
*/
bool SavePng(Gdiplus::Bitmap& bitmap, std::vector<uint8_t>& output)
{
    CLSID clsid{};
    if (GetPngEncoderClsid(clsid) == false) {
        return false;
    }

    IStream* stream = nullptr;
    if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &stream))) {
        return false;
    }
    if (bitmap.Save(stream, &clsid, nullptr) != Gdiplus::Ok) {
        stream->Release();
        return false;
    }

    STATSTG stat{};
    if (FAILED(stream->Stat(&stat, STATFLAG_NONAME))) {
        stream->Release();
        return false;
    }
    ULONG size = static_cast<ULONG>(stat.cbSize.QuadPart);
    output.resize(size);

    LARGE_INTEGER position{};
    if (FAILED(stream->Seek(position, STREAM_SEEK_SET, nullptr))) {
        stream->Release();
        output.clear();
        return false;
    }
    ULONG readBytes = 0;
    HRESULT hr = stream->Read(output.data(), size, &readBytes);
    stream->Release();
    if (FAILED(hr) || readBytes != size) {
        output.clear();
        return false;
    }
    return true;
}

/**
  透過情報を含むテスト用PNGを作成する
  @return true:成功  false:失敗
  @param[out] output 作成したPNGデータ
*/
bool CreateTestImage(std::vector<uint8_t>& output)
{
    Gdiplus::Bitmap bitmap(128, 64, PixelFormat32bppARGB);
    if (bitmap.GetLastStatus() != Gdiplus::Ok) {
        return false;
    }

    Gdiplus::Graphics graphics(&bitmap);
    graphics.Clear(Gdiplus::Color(0, 0, 0, 0));
    Gdiplus::SolidBrush brush(Gdiplus::Color(128, 255, 0, 0));
    graphics.FillRectangle(&brush, 32, 16, 64, 32);
    return SavePng(bitmap, output);
}

class GdiplusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Gdiplus::GdiplusStartupInput input;
        ASSERT_EQ(
            Gdiplus::Ok,
            Gdiplus::GdiplusStartup(&mGdiplusToken, &input, nullptr));
    }

    void TearDown() override
    {
        Gdiplus::GdiplusShutdown(mGdiplusToken);
    }

private:
    ULONG_PTR mGdiplusToken{};
};

} // namespace

TEST_F(GdiplusTest, ConvertResizesImageAndKeepsAlpha)
{
    std::vector<uint8_t> input;
    ASSERT_TRUE(CreateTestImage(input));

    std::vector<uint8_t> output;
    ASSERT_TRUE(launcherapp::utility::ImageConverter::Convert(input, output));

    IStream* stream = nullptr;
    ASSERT_EQ(S_OK, CreateStreamOnHGlobal(nullptr, TRUE, &stream));
    ULONG writtenBytes = 0;
    ASSERT_EQ(S_OK, stream->Write(output.data(), static_cast<ULONG>(output.size()), &writtenBytes));
    ASSERT_EQ(output.size(), writtenBytes);
    LARGE_INTEGER position{};
    ASSERT_EQ(S_OK, stream->Seek(position, STREAM_SEEK_SET, nullptr));

    std::unique_ptr<Gdiplus::Bitmap> result(Gdiplus::Bitmap::FromStream(stream));
    ASSERT_NE(nullptr, result);
    ASSERT_EQ(Gdiplus::Ok, result->GetLastStatus());
    EXPECT_EQ(64u, result->GetWidth());
    EXPECT_EQ(32u, result->GetHeight());

    Gdiplus::Color transparent;
    Gdiplus::Color filled;
    ASSERT_EQ(Gdiplus::Ok, result->GetPixel(0, 0, &transparent));
    ASSERT_EQ(Gdiplus::Ok, result->GetPixel(32, 16, &filled));
    EXPECT_EQ(0, transparent.GetA());
    EXPECT_GT(filled.GetA(), 0);
    EXPECT_EQ(PixelFormat32bppARGB, result->GetPixelFormat());
    result.reset();
    stream->Release();
}

TEST_F(GdiplusTest, ConvertSmallImageStillReturnsPng)
{
    Gdiplus::Bitmap bitmap(16, 16, PixelFormat32bppARGB);
    ASSERT_EQ(Gdiplus::Ok, bitmap.GetLastStatus());

    std::vector<uint8_t> input;
    ASSERT_TRUE(SavePng(bitmap, input));

    std::vector<uint8_t> output;
    ASSERT_TRUE(launcherapp::utility::ImageConverter::Convert(input, output));
    ASSERT_FALSE(output.empty());
    EXPECT_EQ(0x89, output[0]);
    EXPECT_EQ('P', output[1]);
    EXPECT_EQ('N', output[2]);
    EXPECT_EQ('G', output[3]);
}

TEST_F(GdiplusTest, ConvertInvalidDataReturnsFalse)
{
    std::vector<uint8_t> input(10, 0xff);
    std::vector<uint8_t> output;
    EXPECT_FALSE(launcherapp::utility::ImageConverter::Convert(input, output));
    EXPECT_TRUE(output.empty());
}
