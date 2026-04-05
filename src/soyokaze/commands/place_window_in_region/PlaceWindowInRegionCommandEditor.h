#pragma once

#include "commands/core/CommandEditorIF.h"
#include "commands/place_window_in_region/PlaceWindowInRegionParameter.h"

// {0B81A208-5879-4A5D-B77A-C7AB8A8C2E4C}
constexpr launcherapp::core::IFID IFID_PLACEWINDOWINREGIONCOMMANDEDITOR =
{ 0xb81a208, 0x5879, 0x4a5d, { 0xb7, 0x7a, 0xc7, 0xab, 0x8a, 0x8c, 0x2e, 0x4c } };

namespace launcherapp {
namespace commands {
namespace place_window_in_region {

class PlaceWindowInRegionCommandEditor : virtual public launcherapp::core::CommandEditor
{
public:
	PlaceWindowInRegionCommandEditor(CWnd* parentWnd = nullptr);
	~PlaceWindowInRegionCommandEditor();

	void SetParam(const CommandParam& param);
	const CommandParam& GetParam();

// CommandEditor
	// 名前を上書きする
	void OverrideName(LPCTSTR name) override;
	// 元のコマンド名を設定する(そのコマンド名と同じ場合は「コマンド名重複」とみなさない)
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


} // end of namespace place_window_in_region
} // end of namespace commands
} // end of namespace launcherapp

