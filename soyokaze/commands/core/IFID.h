#pragma once

#include <stdint.h>

namespace launcherapp {
namespace core {

struct IFID {
	bool operator == (const IFID& rhs) const {
		return memcmp(this, &rhs, sizeof(IFID)) == 0;
	}
	uint32_t mData1;
	uint16_t mData2;
	uint16_t mData3;
	uint8_t mData4[8];
};

}
}

