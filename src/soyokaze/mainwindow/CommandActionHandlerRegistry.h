#pragma once

#include "mainwindow/CandidateListListenerIF.h"
#include "mainwindow/CandidateList.h"

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

