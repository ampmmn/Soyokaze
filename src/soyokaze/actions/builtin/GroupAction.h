#pragma once

#include "actions/core/ActionBase.h"

namespace launcherapp { namespace actions { namespace builtin {

// $BB>$N%"%/%7%g%s$r$^$H$a$F<B9T$9$k%"%/%7%g%s(B
class GroupAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	GroupAction(const CString& parentName, uint32_t modifierKeyState = 0);
	~GroupAction();

	void AddAction(Action* action);
	void EnableConfirm(bool shouldConfirm);
	void StopIfErrorOccured(bool shouldStop);
	void SetRepeats(uint32_t repeats);
	void EnablePassParam(bool shouldPassParam);
	
// Action
	// $B%"%/%7%g%s$NFbMF$r<($9L>>N(B
	CString GetDisplayName() override;
	// $B%"%/%7%g%s$r<B9T$9$k(B
	bool Perform(Parameter* param, String* errMsg) override;
	// $B%,%$%IMs$J$I$KI=<($9$k$+$I$&$+(B
	bool IsVisible() override;

private:
	bool Confirm(Parameter* param);
	bool BuildSubParameter(Parameter* param, Parameter** paramSub);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}}
