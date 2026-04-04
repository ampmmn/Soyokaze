#pragma once

namespace launcherapp {
namespace commands {
namespace common {

HRESULT AutoWrap(int autoType, VARIANT* pvResult, IDispatch* pDisp, LPCOLESTR ptName, int cArgs...);


}
}
}
