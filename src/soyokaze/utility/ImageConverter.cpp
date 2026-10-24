#include "pch.h"
#include "framework.h"
#include "ImageConverter.h"

#include <gdiplus.h>
#define LUNASVG_BUILD_STATIC
#include <lunasvg.h>
#include <cmath>
#include <memory>

#pragma comment(lib, "Gdiplus.lib")

namespace launcherapp {
namespace utility {

using namespace Gdiplus;

namespace {

struct StreamReleaser
{
    void operator()(IStream* stream) const
    {
        if (stream != nullptr) {
            stream->Release();
        }
    }
};

using StreamPtr = std::unique_ptr<IStream, StreamReleaser>;

/**
  PNGエンコーダーのCLSIDを取得する
  @return true:成功  false:失敗
  @param[out] clsid PNGエンコーダーのCLSID
*/
bool GetPngEncoderClsid(CLSID& clsid)
{
    UINT encoderCount = 0;
    UINT bufferSize = 0;
    if (Gdiplus::GetImageEncodersSize(&encoderCount, &bufferSize) != Gdiplus::Ok ||
        bufferSize == 0) {
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
  メモリ上の画像データをGDI+のBitmapへ読み込む
  @return nullptr:失敗  その他:読み込んだBitmap
  @param[in] data 画像データ
*/
std::unique_ptr<Gdiplus::Bitmap> LoadBitmap(
    const std::vector<uint8_t>& data,
    StreamPtr& streamHolder
)
{
    if (data.empty()) {
        return nullptr;
    }

    HGLOBAL global = GlobalAlloc(GMEM_MOVEABLE, data.size());
    if (global == nullptr) {
        return nullptr;
    }

    void* buffer = GlobalLock(global);
    if (buffer == nullptr) {
        GlobalFree(global);
        return nullptr;
    }
    memcpy(buffer, data.data(), data.size());
    GlobalUnlock(global);

    IStream* stream = nullptr;
    if (FAILED(CreateStreamOnHGlobal(global, TRUE, &stream))) {
        GlobalFree(global);
        return nullptr;
    }
    streamHolder.reset(stream);

    std::unique_ptr<Gdiplus::Bitmap> bitmap(Gdiplus::Bitmap::FromStream(streamHolder.get()));
    if (bitmap == nullptr || bitmap->GetLastStatus() != Gdiplus::Ok) {
        return nullptr;
    }
    return bitmap;
}

/**
  BitmapをPNGデータへ変換する
  @return true:成功  false:失敗
  @param[in] bitmap 変換元のBitmap
  @param[out] output 変換後のPNGデータ
*/
bool SavePng(Gdiplus::Bitmap& bitmap, std::vector<uint8_t>& output)
{
    CLSID pngClsid{};
    if (GetPngEncoderClsid(pngClsid) == false) {
        return false;
    }

    IStream* stream = nullptr;
    if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &stream))) {
        return false;
    }

    Gdiplus::Status status = bitmap.Save(stream, &pngClsid, nullptr);
    if (status != Gdiplus::Ok) {
        stream->Release();
        return false;
    }

    STATSTG stat{};
    if (FAILED(stream->Stat(&stat, STATFLAG_NONAME)) ||
        stat.cbSize.QuadPart > ULONG_MAX) {
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
  SVGデータを指定サイズ以内のPNGデータへ変換する
  @return true:成功  false:失敗
  @param[in] input 変換元のSVGデータ
  @param[out] output 変換後のPNGデータ
  @param[in] maxSize 画像の長辺の最大サイズ
*/
bool ConvertSvg(
    const std::vector<uint8_t>& input,
    std::vector<uint8_t>& output,
    int maxSize
)
{
    auto document = lunasvg::Document::loadFromData(
        reinterpret_cast<const char*>(input.data()),
        input.size());
    if (document == nullptr) {
        return false;
    }

    double sourceWidth = document->width();
    double sourceHeight = document->height();
    if (std::isfinite(sourceWidth) == false || std::isfinite(sourceHeight) == false ||
        sourceWidth <= 0.0 || sourceHeight <= 0.0) {
        return false;
    }

    double scale = 1.0;
    if (sourceWidth > maxSize || sourceHeight > maxSize) {
        double longestSide = sourceWidth >= sourceHeight ? sourceWidth : sourceHeight;
        scale = maxSize / longestSide;
    }

    int width = static_cast<int>(std::round(sourceWidth * scale));
    int height = static_cast<int>(std::round(sourceHeight * scale));
    if (width < 1) {
        width = 1;
    }
    if (height < 1) {
        height = 1;
    }

    // 長辺に合わせた正方形の描画先を作り、SVG本体を中央に配置する
    int canvasSize = width >= height ? width : height;
    lunasvg::Bitmap svgBitmap(canvasSize, canvasSize);
    if (svgBitmap.isNull()) {
        return false;
    }

    float scaleX = width / static_cast<float>(sourceWidth);
    float scaleY = height / static_cast<float>(sourceHeight);
    float offsetX = (canvasSize - width) / 2.0f;
    float offsetY = (canvasSize - height) / 2.0f;
    document->render(svgBitmap, lunasvg::Matrix(scaleX, 0.0f, 0.0f, scaleY, offsetX, offsetY));

    // LunaSVGのARGB32 premultiplied形式をGDI+の対応形式としてPNGへ保存する
    Gdiplus::Bitmap result(
        svgBitmap.width(),
        svgBitmap.height(),
        svgBitmap.stride(),
        PixelFormat32bppPARGB,
        svgBitmap.data());
    if (result.GetLastStatus() != Gdiplus::Ok) {
        return false;
    }

    return SavePng(result, output);
}

} // namespace

bool ImageConverter::Convert(
    const std::vector<uint8_t>& input,
    std::vector<uint8_t>& output,
    int maxSize
)
{
    output.clear();
    if (input.empty() || maxSize <= 0) {
        return false;
    }

    StreamPtr sourceStream;
    auto source = LoadBitmap(input, sourceStream);
    if (source == nullptr) {
        return ConvertSvg(input, output, maxSize);
    }

    UINT sourceWidth = source->GetWidth();
    UINT sourceHeight = source->GetHeight();
    if (sourceWidth == 0 || sourceHeight == 0) {
        return false;
    }

    int width = static_cast<int>(sourceWidth);
    int height = static_cast<int>(sourceHeight);
    if (sourceWidth > static_cast<UINT>(maxSize) ||
        sourceHeight > static_cast<UINT>(maxSize)) {
        if (sourceWidth >= sourceHeight) {
            width = maxSize;
            height = static_cast<int>(sourceHeight * (maxSize / static_cast<double>(sourceWidth)));
        }
        else {
            width = static_cast<int>(sourceWidth * (maxSize / static_cast<double>(sourceHeight)));
            height = maxSize;
        }
        if (width < 1) {
            width = 1;
        }
        if (height < 1) {
            height = 1;
        }
    }

    Gdiplus::Bitmap result(width, height, PixelFormat32bppARGB);
    if (result.GetLastStatus() != Gdiplus::Ok) {
        return false;
    }

    Gdiplus::Graphics graphics(&result);
    if (graphics.GetLastStatus() != Gdiplus::Ok) {
        return false;
    }
    graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
    graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.Clear(Gdiplus::Color(0, 0, 0, 0));
    if (graphics.DrawImage(source.get(), 0, 0, width, height) != Gdiplus::Ok) {
        return false;
    }

    return SavePng(result, output);
}

} // namespace utility
} // namespace launcherapp
