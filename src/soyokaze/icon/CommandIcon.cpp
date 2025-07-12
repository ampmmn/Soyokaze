#include "pch.h"
#include "CommandIcon.h"
#include "icon/IconLoader.h"
#include "resource.h"

namespace launcherapp { namespace icon {

enum IconType {
	ICONTYPE_FROMPATH,
	ICONTYPE_FROMSTREAM,
	ICONTYPE_NULL,
};

struct CommandIcon::PImpl
{
	HICON FetchIcon()
	{
		auto iconLoader = IconLoader::Get();

		// 既にアイコンがロードずみで、それがまだ有効ならそれを使う
		if (mIconHandle && iconLoader->HasIcon(mIconHandle)) {
			return mIconHandle;
		}

		// タイプに応じてアイコンを取得する
		if (mType == ICONTYPE_FROMPATH) {
			mIconHandle = iconLoader->LoadIconFromPath(mFilePath);
		}
		else if (mType == ICONTYPE_FROMSTREAM) {
			mIconHandle = iconLoader->LoadIconFromStream(mStream);
		}

		// アイコンをロードできていたらそれを返す、失敗していたらデフォルトアイコンを返す
		return mIconHandle ? mIconHandle :iconLoader->LoadDefaultIcon();
	}

	HICON mIconHandle{nullptr};
	bool mIsLoaded{false};
	CString mFilePath;
	std::vector<uint8_t> mStream;

	IconType mType{ICONTYPE_NULL};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CommandIcon::CommandIcon() : in(new PImpl)
{
}

CommandIcon::~CommandIcon()
{
}

CommandIcon::operator HICON()
{
	return IconHandle();
}

/**
	アイコンハンドルを取得する
	@return アイコンハンドル
*/
HICON CommandIcon::IconHandle()
{
	return in->FetchIcon();
}

bool CommandIcon::IsNull()
{
	return in->mType == ICONTYPE_NULL;
}

void CommandIcon::Reset()
{
	if (in->mIconHandle == nullptr) {
		return;
	}

	// アイコンロード時の情報をクリアする
	in->mIconHandle = nullptr;
	in->mType = ICONTYPE_NULL;
	in->mFilePath.Empty();
	in->mStream.clear();
}

HICON CommandIcon::LoadFromPath(LPCTSTR path)
{
	Reset();

	in->mType = ICONTYPE_FROMPATH;
	in->mFilePath = path;
	return in->FetchIcon();
}

HICON CommandIcon::LoadFromStream(const std::vector<uint8_t>& stm)
{
	Reset();

	in->mStream = stm;
	in->mType = ICONTYPE_FROMSTREAM;
	return in->FetchIcon();
}


}}

