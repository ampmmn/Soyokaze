#pragma once

#include "core/UnknownIF.h"

namespace launcherapp {
namespace core {

class CommandEditor : virtual public UnknownIF
{
public:
	// $BL>A0$r>e=q$-$9$k(B
	virtual void OverrideName(LPCTSTR name) = 0;
	// $B85$N%3%^%s%IL>$r@_Dj$9$k(B($B$=$N%3%^%s%IL>$HF1$8>l9g$O!V%3%^%s%IL>=EJ#!W$H$_$J$5$J$$(B)
	virtual void SetOriginalName(LPCTSTR name) = 0;
	// $B%3%^%s%I$rJT=8$9$k$?$a$N%@%$%"%m%0$r:n@.(B/$B<hF@$9$k(B
	virtual bool DoModal() = 0;
};

}  // end of namespace core
}  // end of namespace launcherapp



