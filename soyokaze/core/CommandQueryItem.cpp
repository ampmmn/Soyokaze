#include "pch.h"
#include "CommandQueryItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CommandQueryItem::CommandQueryItem(
	int level,
	soyokaze::core::Command* cmd
) : 
	mMatchLevel(level), mCommand(cmd)
{
}



