#include "pch.h"
#include "OutputPreviewEditor.h"
#include "OutputPreviewSettingDialog.h"

namespace launcherapp {
namespace commands {
namespace output_preview {

struct OutputPreviewEditor::PImpl
{
	explicit PImpl(CWnd* parentWnd) : mDialog(parentWnd)
	{
	}
	OutputPreviewSettingDialog mDialog;
	uint32_t mRefCount{1};
};

OutputPreviewEditor::OutputPreviewEditor(CWnd* parentWnd) : in(std::make_unique<PImpl>(parentWnd))
{
}

OutputPreviewEditor::~OutputPreviewEditor()
{
}

void OutputPreviewEditor::SetParam(const OutputPreviewParam& param)
{
	in->mDialog.SetParam(param);
}

const OutputPreviewParam& OutputPreviewEditor::GetParam() const
{
	return in->mDialog.GetParam();
}

void OutputPreviewEditor::OverrideName(LPCTSTR name)
{
	OutputPreviewParam param = in->mDialog.GetParam();
	param.mName = name;
	in->mDialog.SetParam(param);
}

void OutputPreviewEditor::SetOriginalName(LPCTSTR name)
{
	in->mDialog.SetOriginalName(name);
}

bool OutputPreviewEditor::DoModal()
{
	return in->mDialog.DoModal() == IDOK;
}

bool OutputPreviewEditor::QueryInterface(const launcherapp::core::IFID& ifid, void** obj)
{
	if (ifid != IFID_OUTPUTPREVIEWEDITOR) {
		return false;
	}
	*obj = static_cast<OutputPreviewEditor*>(this);
	AddRef();
	return true;
}

uint32_t OutputPreviewEditor::AddRef()
{
	return (uint32_t)InterlockedIncrement((LONG*)&in->mRefCount);
}

uint32_t OutputPreviewEditor::Release()
{
	auto count = (uint32_t)InterlockedDecrement((LONG*)&in->mRefCount);
	if (count == 0) {
		delete this;
	}
	return count;
}

}
}
}
