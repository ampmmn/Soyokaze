#include "pch.h"
#include "BrowserProfile.h"
#include "utility/Path.h"

#include <fstream>

#pragma warning( push )
#pragma warning( disable : 26800 26819 )
#include <nlohmann/json.hpp>
#pragma warning( pop )

namespace launcherapp { namespace externaltool { namespace webbrowser {

// Local Stateの読み取りに失敗した場合はDefaultを使用する
CString BrowserProfile::GetLastUsedProfileName(LPCTSTR userDataPath)
{
	CString profileName(_T("Default"));

	Path localStatePath(userDataPath);
	localStatePath.Append(_T("Local State"));

	try {
		std::ifstream file((LPCTSTR)localStatePath);
		if (file.is_open() == false) {
			return profileName;
		}

		auto localState = nlohmann::json::parse(file);
		auto profile = localState.find("profile");
		if (profile == localState.end() || profile->is_object() == false) {
			return profileName;
		}

		auto lastUsed = profile->find("last_used");
		if (lastUsed == profile->end() || lastUsed->is_string() == false) {
			return profileName;
		}

		CString resolvedName;
		UTF2UTF(lastUsed->get<std::string>(), resolvedName);
		if (resolvedName.IsEmpty() == false) {
			profileName = resolvedName;
		}
	}
	catch (const nlohmann::json::exception&) {
		return profileName;
	}
	catch (const std::exception&) {
		return profileName;
	}

	return profileName;
}

}}}
