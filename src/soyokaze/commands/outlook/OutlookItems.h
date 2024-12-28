#pragma once

#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace outlook {

class MailItem;

class OutlookItems
{
public:
	OutlookItems();
	~OutlookItems();

public:
	bool GetInboxMailItems(std::vector<MailItem*>& mailItems);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


class MailItem
{
public:
	MailItem(const CString& conversationId, const CString& subject);
	~MailItem();

	const CString& GetConversationID();
	const CString& GetSubject();

	BOOL Activate(bool isShowMaximize = false);

	uint32_t AddRef();
	uint32_t Release();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



} // end of namespace outlook
} // end of namespace commands
} // end of namespace launcherapp

