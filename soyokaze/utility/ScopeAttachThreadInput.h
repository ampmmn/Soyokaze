#pragma once


class ScopeAttachThreadInput
{
public:
	ScopeAttachThreadInput() : 
		target(GetWindowThreadProcessId(::GetForegroundWindow(),NULL))
 	{
		AttachThreadInput(GetCurrentThreadId(), target, TRUE);
	}

	ScopeAttachThreadInput(DWORD target) : target(target) {
		AttachThreadInput(GetCurrentThreadId(), target, TRUE);
	}

	~ScopeAttachThreadInput()
	{
		AttachThreadInput(GetCurrentThreadId(), target, FALSE);
	}

	DWORD target;
};
