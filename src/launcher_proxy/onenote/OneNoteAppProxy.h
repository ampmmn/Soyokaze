#pragma once

#include <memory>

#pragma warning( push )
#pragma warning( disable : 26800 26819 )
#include <nlohmann/json.hpp>
#pragma warning( pop )


namespace launcherproxy { 

class OneNoteAppProxy
{
	OneNoteAppProxy();
	~OneNoteAppProxy();

public:
	// インスタンスを取得
	static OneNoteAppProxy* GetInstance();

	void Abort();

	// ブック一覧を取得する
	bool GetHierarchy(nlohmann::json& noteBooks);
	//
	std::string GetErrorMessage();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace 
