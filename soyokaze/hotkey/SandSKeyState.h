#pragma once

#include <memory>

class SandSKeyState
{
	SandSKeyState();
	~SandSKeyState();
	
public:
	void Initialize();
	void Finalize();

	bool IsPressed(UINT modKeyCode, UINT keyCode);
	
	static SandSKeyState* GetInstance();
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

