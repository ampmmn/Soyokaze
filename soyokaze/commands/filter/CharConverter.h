#pragma once

namespace soyokaze {
namespace commands {
namespace filter {

class CharConverter
{
public:
	CharConverter();
	~CharConverter();

	CString& Convert(const char* src, CString& dst);


protected:
	CStringW mBuff;

};


} // end of namespace filter
} // end of namespace commands
} // end of namespace soyokaze

