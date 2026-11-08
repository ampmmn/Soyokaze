#include "pch.h"
#include "OutputPreviewRuntime.h"

namespace launcherapp {
namespace commands {
namespace output_preview {

struct OutputPreviewRuntime::PImpl
{
	mutable std::mutex mMutex;
	std::condition_variable mCondition;
	CString mResultName;
	CString mCommandName;
	CString mInputText;
	CString mExecutablePath;
	CString mText;
	CString mFilePath;
	std::vector<CString> mTemporaryFiles;
	int mPreviewType{0};
	bool mIsRunning{false};
	bool mHasResult{false};
};

OutputPreviewRuntime::OutputPreviewRuntime() : in(std::make_unique<PImpl>())
{
}

OutputPreviewRuntime::~OutputPreviewRuntime()
{
	std::unique_lock<std::mutex> lock(in->mMutex);
	in->mCondition.wait(lock, [this]() { return in->mIsRunning == false; });
	CString resultName = in->mResultName;
	lock.unlock();
	if (resultName.IsEmpty() == FALSE) {
		Close(resultName);
	}
}

OutputPreviewRuntime* OutputPreviewRuntime::Get()
{
	static OutputPreviewRuntime runtime;
	return &runtime;
}

bool OutputPreviewRuntime::TryBegin(
	const CString& resultName,
	const CString& commandName,
	const CString& inputText
)
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	if (in->mResultName.IsEmpty() == FALSE) {
		return false;
	}
	in->mResultName = resultName;
	in->mCommandName = commandName;
	in->mInputText = inputText;
	in->mIsRunning = true;
	in->mHasResult = false;
	return true;
}

bool OutputPreviewRuntime::AddTemporaryFile(const CString& resultName, const CString& path)
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	if (in->mResultName != resultName || in->mIsRunning == false) {
		return false;
	}
	in->mTemporaryFiles.push_back(path);
	return true;
}

bool OutputPreviewRuntime::Publish(
	const CString& resultName,
	const CString& text,
	const CString& filePath,
	int previewType,
	const CString& executablePath
)
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	if (in->mResultName != resultName || in->mIsRunning == false) {
		return false;
	}
	in->mText = text;
	in->mFilePath = filePath;
	in->mExecutablePath = executablePath;
	in->mPreviewType = previewType;
	in->mIsRunning = false;
	in->mHasResult = true;
	in->mCondition.notify_all();
	return true;
}

bool OutputPreviewRuntime::Fetch(const CString& resultName, Result& result)
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	if (in->mResultName != resultName || in->mHasResult == false) {
		return false;
	}
	result.mName = in->mResultName;
	result.mCommandName = in->mCommandName;
	result.mInputText = in->mInputText;
	result.mExecutablePath = in->mExecutablePath;
	result.mText = in->mText;
	result.mFilePath = in->mFilePath;
	result.mPreviewType = in->mPreviewType;
	return true;
}

bool OutputPreviewRuntime::GetResultName(CString& resultName) const
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	if (in->mHasResult == false) {
		return false;
	}
	resultName = in->mResultName;
	return true;
}

bool OutputPreviewRuntime::IsResultForCommand(const CString& commandName, const CString& inputText) const
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	return in->mHasResult && in->mCommandName.CompareNoCase(commandName) == 0 &&
		in->mInputText.CompareNoCase(inputText) == 0;
}

void OutputPreviewRuntime::Close(const CString& resultName)
{
	std::vector<CString> temporaryFiles;
	{
		std::lock_guard<std::mutex> lock(in->mMutex);
		if (in->mResultName != resultName) {
			return;
		}
		temporaryFiles.swap(in->mTemporaryFiles);
		in->mResultName.Empty();
		in->mCommandName.Empty();
		in->mInputText.Empty();
		in->mExecutablePath.Empty();
		in->mText.Empty();
		in->mFilePath.Empty();
		in->mPreviewType = 0;
		in->mIsRunning = false;
		in->mHasResult = false;
		in->mCondition.notify_all();
	}

	for (const auto& path : temporaryFiles) {
		DeleteFile(path);
	}
}

bool OutputPreviewRuntime::IsBusy() const
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	return in->mResultName.IsEmpty() == FALSE;
}

}
}
}
