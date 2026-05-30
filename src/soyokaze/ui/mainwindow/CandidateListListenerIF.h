#pragma once

class CandidateListListenerIF
{
public:
	virtual ~CandidateListListenerIF() {}

	virtual void OnUpdateSelect(void* sender) = 0;
	virtual void OnUpdateItems(void* sender) = 0;
};

