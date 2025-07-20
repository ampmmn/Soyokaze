#pragma once

#include "commands/core/CommandQueryResult.h"

namespace launcherapp { namespace commands { namespace core {

// $B%3%^%s%I$NLd$$9g$o$;MW5a$N$?$a$N%$%s%?%U%'!<%9(B
class CommandQueryRequest
{
public:
	virtual ~CommandQueryRequest() {}

	// $B8!:w%-!<%o!<%I(B($BJ8;zNsA4BN(B)$B$r<hF@$9$k(B
	virtual CString GetCommandParameter() = 0;
	// $B8!:w7k2L$rDLCN$9$k$?$a$N%3!<%k%P%C%/4X?t(B
	virtual void NotifyQueryComplete(bool isCancelled, CommandQueryResult* result) = 0;
	// $B;2>H%+%&%s%H$r>e$2$k(B
	virtual uint32_t AddRef() = 0;
	// $B;2>H%+%&%s%H$r2<$2$k(B
	virtual uint32_t Release() = 0;
};

}}}

