#pragma once

#include <stdint.h>

namespace launcherapp {
namespace core {

struct IFID {
	uint32_t mData1;
	uint16_t mData2;
	uint16_t mData3;
	uint8_t mData4[8];
};

}
}

