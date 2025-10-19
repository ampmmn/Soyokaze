#pragma once

#include <memory>
#include "ILexer.h"  // scintilla

namespace launcherapp { namespace commands { namespace py_extension {

class ScintillaDLLLoader
{
private:
	ScintillaDLLLoader();
	~ScintillaDLLLoader();

public:
	static ScintillaDLLLoader* GetInstance();

	bool Initialize();
private:
	// Finalizeは内部で自動でやるので外からは呼ぶ必要がない
	bool Finalize();

public:
	Scintilla::ILexer5* CreateLexer(const char* langName);


private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}}
