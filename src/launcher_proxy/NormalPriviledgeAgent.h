#pragma once

#include <memory>
#include <string>

class NormalPriviledgeAgent
{
public:
	NormalPriviledgeAgent();
	~NormalPriviledgeAgent();

	int Run(HINSTANCE hInst);

private:
	void ProcRequest(const std::string& request);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

