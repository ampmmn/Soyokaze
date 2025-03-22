#pragma once

#include <stdint.h>
#include <vector>

namespace utility {
namespace base64 {

CString EncodeBase64(const std::vector<uint8_t>& stm);
bool DecodeBase64(const CString& src, std::vector<uint8_t>& stm);


}
}
