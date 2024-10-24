// あ
#pragma once

class CommandEntryIF;

namespace launcherapp {
namespace commands {
namespace watchpath {

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam&) = default;
	~CommandParam();

	bool operator == (const CommandParam& rhs) const;

	bool Save(CommandEntryIF* entry);
	bool Load(CommandEntryIF* entry);

	void swap(CommandParam& rhs);

public:
	CString mName;
	CString mDescription;
	CString mPath;
	CString mNotifyMessage;
	bool mIsDisabled;
};


}
}
}

