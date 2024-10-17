#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/ejectvolume/EjectVolumeCommandParam.h"

// {D2A4BBAC-7EA9-497B-9590-984E008ACCEA}
constexpr launcherapp::core::IFID IFID_EJECTVOLUMECOMMANDEDITOR =
{ 0xd2a4bbac, 0x7ea9, 0x497b, { 0x95, 0x90, 0x98, 0x4e, 0x0, 0x8a, 0xcc, 0xea } };

namespace launcherapp {
namespace commands {
namespace ejectvolume {

class EjectVolumeCommandEditor : virtual public launcherapp::core::CommandEditor
{
public:
	EjectVolumeCommandEditor(CWnd* parentWnd = nullptr);
	~EjectVolumeCommandEditor();

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


} // end of namespace ejectvolume
} // end of namespace commands
} // end of namespace launcherapp

