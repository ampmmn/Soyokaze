#pragma once

#include "features/main/CandidateListListenerIF.h"
#include "features/main/CandidateList.h"

namespace launcherapp { namespace mainwindow {

class CommandActionHandlerRegistry : public CandidateListListenerIF
{
public:
	CommandActionHandlerRegistry();
	~CommandActionHandlerRegistry();

	void Initialize(CandidateList* candidateList);
	void Finalize(CandidateList* candidateList);

// CandidateListListenerIF
	void OnUpdateSelect(void* sender) override;
	void OnUpdateItems(void* sender) override;
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}

