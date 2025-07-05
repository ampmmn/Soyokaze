#pragma once

namespace launcherapp {
namespace commands {
namespace common {

HRESULT AutoWrap(int autoType, VARIANT* pvResult, IDispatch* pDisp, LPOLESTR ptName, int cArgs...);


}
}
}
