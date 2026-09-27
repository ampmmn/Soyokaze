#pragma once

#include <memory>
#include <vector>

namespace launcherapp { namespace core {
class Command;
}}

class ImportCommandsDialog : public CDialogEx
{
public:
	ImportCommandsDialog();
	virtual ~ImportCommandsDialog();

	void SetCommands(const std::vector<launcherapp::core::Command*>& commands);
	std::vector<int> GetSelectedIndices() const;
	bool IsOverwriteSelected() const;

	static bool CanImport(int n);

protected:
	void UpdateStatus();

	BOOL OnInitDialog() override;
	void OnOK() override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLvnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
