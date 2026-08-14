#pragma once

#include <cstdint>
#include <vector>

namespace launcherapp {
namespace utility {

class ImageConverter
{
public:
    /**
      画像データを指定サイズ以内の32bpp ARGB PNGへ変換する
      @return true:成功  false:失敗
      @param[in] input 変換元の画像データ
      @param[out] output 変換後のPNGデータ
      @param[in] maxSize 画像の長辺の最大サイズ
    */
    static bool Convert(
        const std::vector<uint8_t>& input,
        std::vector<uint8_t>& output,
        int maxSize = 64
    );
};

} // namespace utility
} // namespace launcherapp
