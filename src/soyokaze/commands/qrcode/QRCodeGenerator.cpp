#include "pch.h"
#include "QRCodeGenerator.h"
#include "SharedHwnd.h"
#include <cstdint>
#include <exception>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <fstream>
#include <filesystem>
#include "QR-Code-generator-1.8.0/cpp/qrcodegen.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace qrcode {

namespace {

constexpr int MODULE_SCALE = 4;
constexpr int QUIET_ZONE_MODULES = 4;

/**
  Unicode文字列をUTF-8へ変換する
  @return true:成功  false:変換失敗
  @param[in] text 変換元文字列
  @param[out] utf8 変換後の文字列
*/
bool ConvertToUtf8(const CString& text, std::string& utf8)
{
	utf8.clear();
	if (text.IsEmpty()) {
		return false;
	}

	int requiredSize = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
		text, text.GetLength(), nullptr, 0, nullptr, nullptr);
	if (requiredSize <= 0) {
		return false;
	}

	utf8.resize(requiredSize);
	int writtenSize = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
		text, text.GetLength(), utf8.data(), requiredSize, nullptr, nullptr);
	if (writtenSize != requiredSize) {
		utf8.clear();
		return false;
	}
	return true;
}

/**
  誤り訂正レベルをNayukiライブラリの値へ変換する
  @return 変換後のレベル。範囲外の場合はfalse
  @param[in] level 設定値
  @param[out] result 変換後のレベル
*/
bool ConvertLevel(int level, ::qrcodegen::QrCode::Ecc& result)
{
	switch (level) {
	case QRCodeGenerator::LEVEL_L:
		result = ::qrcodegen::QrCode::Ecc::LOW;
		return true;
	case QRCodeGenerator::LEVEL_M:
		result = ::qrcodegen::QrCode::Ecc::MEDIUM;
		return true;
	case QRCodeGenerator::LEVEL_Q:
		result = ::qrcodegen::QrCode::Ecc::QUARTILE;
		return true;
	case QRCodeGenerator::LEVEL_H:
		result = ::qrcodegen::QrCode::Ecc::HIGH;
		return true;
	default:
		return false;
	}
}

/**
  ビットマップをCF_DIB形式のクリップボードデータへ変換する
  @return nullptr:失敗  その他:生成したHGLOBAL
  @param[in] bitmap 変換元のビットマップ
*/
HGLOBAL CreateDibClipboardData(HBITMAP bitmap)
{
	BITMAP bitmapInfo{};
	if (GetObject(bitmap, sizeof(bitmapInfo), &bitmapInfo) != sizeof(bitmapInfo) ||
		bitmapInfo.bmWidth <= 0 || bitmapInfo.bmHeight <= 0) {
		return nullptr;
	}

	const size_t width = static_cast<size_t>(bitmapInfo.bmWidth);
	const size_t height = static_cast<size_t>(bitmapInfo.bmHeight);
	if (width > ((std::numeric_limits<size_t>::max)() - 3) / 3) {
		return nullptr;
	}
	const size_t rowBytes = (width * 3 + 3) & ~static_cast<size_t>(3);
	if (rowBytes > (std::numeric_limits<size_t>::max)() / height) {
		return nullptr;
	}
	const size_t pixelBytes = rowBytes * height;
	if (pixelBytes > (std::numeric_limits<DWORD>::max)() ||
		pixelBytes > (std::numeric_limits<size_t>::max)() - sizeof(BITMAPINFOHEADER)) {
		return nullptr;
	}

	const size_t dataSize = sizeof(BITMAPINFOHEADER) + pixelBytes;
	HGLOBAL dibData = GlobalAlloc(GMEM_MOVEABLE, dataSize);
	if (dibData == nullptr) {
		return nullptr;
	}

	auto header = static_cast<BITMAPINFOHEADER*>(GlobalLock(dibData));
	if (header == nullptr) {
		GlobalFree(dibData);
		return nullptr;
	}
	*header = {};
	header->biSize = sizeof(BITMAPINFOHEADER);
	header->biWidth = bitmapInfo.bmWidth;
	header->biHeight = bitmapInfo.bmHeight;
	header->biPlanes = 1;
	header->biBitCount = 24;
	header->biCompression = BI_RGB;
	header->biSizeImage = static_cast<DWORD>(pixelBytes);

	BITMAPINFO dibInfo{};
	dibInfo.bmiHeader = *header;
	HDC screenDC = GetDC(nullptr);
	if (screenDC == nullptr) {
		GlobalUnlock(dibData);
		GlobalFree(dibData);
		return nullptr;
	}

	BYTE* pixelData = reinterpret_cast<BYTE*>(header + 1);
	int copiedLines = GetDIBits(screenDC, bitmap, 0, bitmapInfo.bmHeight,
		pixelData, &dibInfo, DIB_RGB_COLORS);
	ReleaseDC(nullptr, screenDC);
	if (copiedLines != bitmapInfo.bmHeight) {
		GlobalUnlock(dibData);
		GlobalFree(dibData);
		return nullptr;
	}

	GlobalUnlock(dibData);
	return dibData;
}

} // 無名名前空間

struct QRCodeGenerator::PImpl
{
	ATL::CImage mImage;
	std::string mSvg;
};

QRCodeGenerator::QRCodeGenerator() : in(std::make_unique<PImpl>())
{
}

QRCodeGenerator::~QRCodeGenerator()
{
}

bool QRCodeGenerator::Generate(const CString& text, int level)
{
	in->mImage.Destroy();
	in->mSvg.clear();

	std::string utf8;
	::qrcodegen::QrCode::Ecc errorCorrection;
	if (ConvertLevel(level, errorCorrection) == false) {
		return false;
	}

	try {
		if (ConvertToUtf8(text, utf8) == false) {
			return false;
		}
	}
	catch (const std::exception&) {
		return false;
	}

	std::optional<::qrcodegen::QrCode> qrCode;
	std::string svgText;
	int moduleCount = 0;
	try {
		// UTF-8のバイト列にECI 26を付け、設定した誤り訂正レベルを維持する
		std::vector<std::uint8_t> utf8Bytes;
		utf8Bytes.reserve(utf8.size());
		for (unsigned char byte : utf8) {
			utf8Bytes.push_back(byte);
		}

		std::vector<::qrcodegen::QrSegment> segments;
		segments.push_back(::qrcodegen::QrSegment::makeEci(26));
		segments.push_back(::qrcodegen::QrSegment::makeBytes(utf8Bytes));
		qrCode.emplace(::qrcodegen::QrCode::encodeSegments(
			segments, errorCorrection, ::qrcodegen::QrCode::MIN_VERSION,
			::qrcodegen::QrCode::MAX_VERSION, -1, false));

		moduleCount = qrCode->getSize();
		const int svgSize = moduleCount + QUIET_ZONE_MODULES * 2;
		std::ostringstream svg;
		svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << svgSize
			<< "\" height=\"" << svgSize << "\" viewBox=\"0 0 " << svgSize << " " << svgSize << "\">\n"
			<< "<rect width=\"100%\" height=\"100%\" fill=\"#fff\"/>\n<path fill=\"#000\" d=\"";
		for (int y = 0; y < moduleCount; ++y) {
			for (int x = 0; x < moduleCount; ++x) {
				if (qrCode->getModule(x, y)) {
					svg << "M" << x + QUIET_ZONE_MODULES << " " << y + QUIET_ZONE_MODULES << "h1v1h-1z";
				}
			}
		}
		svg << "\"/>\n</svg>\n";
		svgText = svg.str();
	}
	catch (const std::exception&) {
		return false;
	}

	int imageSize = (moduleCount + QUIET_ZONE_MODULES * 2) * MODULE_SCALE;
	HRESULT imageStatus = in->mImage.Create(imageSize, imageSize, 24);
	if (FAILED(imageStatus) || in->mImage.IsNull()) {
		return false;
	}

	HDC dc = in->mImage.GetDC();
	if (dc == nullptr) {
		in->mImage.Destroy();
		return false;
	}
	RECT imageRect{0, 0, imageSize, imageSize};
	FillRect(dc, &imageRect, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));

	for (int y = 0; y < moduleCount; ++y) {
		for (int x = 0; x < moduleCount; ++x) {
			if (qrCode->getModule(x, y)) {
				int left = (x + QUIET_ZONE_MODULES) * MODULE_SCALE;
				int top = (y + QUIET_ZONE_MODULES) * MODULE_SCALE;
				RECT moduleRect{left, top, left + MODULE_SCALE, top + MODULE_SCALE};
				FillRect(dc, &moduleRect, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
			}
		}
	}
	in->mImage.ReleaseDC();

	in->mSvg = std::move(svgText);
	return true;
}

bool QRCodeGenerator::CopyToClipboard() const
{
	if (IsValid() == false) {
		return false;
	}

	HBITMAP bitmap = static_cast<HBITMAP>(in->mImage);
	HGLOBAL dibData = CreateDibClipboardData(bitmap);
	if (dibData == nullptr) {
		return false;
	}

	SharedHwnd owner;
	if (OpenClipboard(owner.GetHwnd()) == FALSE) {
		GlobalFree(dibData);
		return false;
	}
	if (EmptyClipboard() == FALSE) {
		CloseClipboard();
		GlobalFree(dibData);
		return false;
	}
	HANDLE result = SetClipboardData(CF_DIB, dibData);
	CloseClipboard();
	if (result == nullptr) {
		GlobalFree(dibData);
		return false;
	}
	return true;
}

bool QRCodeGenerator::SaveImage(LPCTSTR path) const
{
	return IsValid() && SUCCEEDED(in->mImage.Save(path));
}

bool QRCodeGenerator::SaveSvg(LPCTSTR path) const
{
	if (in->mSvg.empty()) {
		return false;
	}
	std::ofstream stream(std::filesystem::path(path), std::ios::binary | std::ios::trunc);
	if (stream.is_open() == false) {
		return false;
	}
	stream.write(in->mSvg.data(), static_cast<std::streamsize>(in->mSvg.size()));
	return stream.good();
}

void QRCodeGenerator::Draw(HDC dc, const RECT& rect) const
{
	if (IsValid()) {
		in->mImage.Draw(dc, rect);
	}
}

bool QRCodeGenerator::IsValid() const
{
	return in->mImage.IsNull() == FALSE;
}

} // qrcode名前空間
} // commands名前空間
} // launcherapp名前空間
