#pragma once

#include "commands/core/CommandIF.h"

namespace launcherapp { namespace commands { namespace core {

	// $B%3%^%s%I8!:w7k2L$N$?$a$N%$%s%?%U%'!<%9(B
class CommandQueryResult
{
public:
	// $B8!:w$K%R%C%H$7$?%3%^%s%I?t$r<hF@$9$k(B
	virtual size_t GetCount() = 0;
	// $B8!:w7k2L?t$,(B0$B$+$I$&$+$r<hF@$9$k(B
	virtual bool IsEmpty() = 0;
	// $B7k2L$r<hF@$9$k(B($B;2>H%+%&%s%H$O8F$S=P$785$G(B-1$B$9$kI,MW$"$j(B)
	virtual bool Get(size_t index, launcherapp::core::Command** cmd, int* matchLevel) = 0;
	// $B7k2L$r<hF@$9$k(B($B;2>H%+%&%s%H$O8F$S=P$785$G(B-1$B$9$kI,MW$"$j(B)
	virtual launcherapp::core::Command* GetItem(size_t index, int* matchLevel = nullptr) = 0;
	// $B;2>H%+%&%s%H$r>e$2$k(B
	virtual uint32_t AddRef() = 0;
	// $B;2>H%+%&%s%H$r2<$2$k(B
	virtual uint32_t Release() = 0;
};

}}}

