#include "pch.h"
#include "SpecialFolderFile.h"

namespace soyokaze {
namespace commands {
namespace specialfolderfiles {


int ITEM::GetTypeFromCSIDL(int csidl)
{
 if (csidl == CSIDL_RECENT) {
	return TYPE_RECENT;
 }
 if (csidl == CSIDL_STARTMENU || csidl == CSIDL_COMMON_STARTMENU) {
	return TYPE_STARTMENU;
 }
 //if (csidl == CSIDL_DESKTOP || csidl =- CSIDL_COMMON_DESKTOPDIRECTORY) {
 else {
	return TYPE_DESKTOP;
 }
}

} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace soyokaze

