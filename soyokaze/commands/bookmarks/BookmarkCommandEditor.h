#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/bookmarks/BookmarkCommandParam.h"


// {41F48C41-E90B-436D-AF61-C3B24B73E5B0}
constexpr launcherapp::core::IFID IFID_BOOKMARKCOMMANDEDITOR =
{ 0x41f48c41, 0xe90b, 0x436d, { 0xaf, 0x61, 0xc3, 0xb2, 0x4b, 0x73, 0xe5, 0xb0 } };

namespace launcherapp {
namespace commands {
namespace bookmarks {

class BookmarkCommandEditor : virtual public launcherapp::core::CommandEditor
{
public:
	BookmarkCommandEditor(CWnd* parentWnd = nullptr);
	~BookmarkCommandEditor();

	void SetParam(const CommandParam& param);
	const CommandParam& GetParam();

// CommandEditor
	// コマンドは編集可能か?
	void SetOriginalName(LPCTSTR name) override;
	// コマンドを編集するためのダイアログを作成/取得する
	bool DoModal() override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** obj) override;
	uint32_t AddRef() override;
	uint32_t Release() override;
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

