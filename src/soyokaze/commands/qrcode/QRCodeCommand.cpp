#include "pch.h"
#include "QRCodeCommand.h"
#include "QRCodeGenerator.h"
#include "QRCodePreviewWindow.h"
#include "actions/core/ActionBase.h"
#include "core/IFIDDefine.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <atlimage.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace qrcode {

namespace {

constexpr LAUNCHER_IFID IFID_QRCODECOMMAND =
	{ 0x15eb6ed5, 0xc01b, 0x49f9, { 0x93, 0x12, 0x8e, 0x18, 0x59, 0xa4, 0x2f, 0x71 } };

/**
  QRコード生成アクション
*/
class QRCodeAction : public launcherapp::actions::core::ActionBase
{
public:
	/**
	  QRコード出力アクションを初期化する
	  @param[in] text QRコードに格納する文字列
	  @param[in] errorCorrectionLevel 誤り訂正レベル
	  @param[in] shouldSave true:ファイルへ保存  false:クリップボードへコピー
	*/
	QRCodeAction(const CString& text, int errorCorrectionLevel, bool shouldSave) :
		mText(text), mErrorCorrectionLevel(errorCorrectionLevel), mShouldSave(shouldSave)
	{
	}

	/**
	  アクション名を取得する
	  @return アクション名
	*/
	CString GetDisplayName() override
	{
		return mShouldSave ? _T("QRコードをファイルに保存") : _T("QRコードをクリップボードにコピー");
	}

	/**
	  QRコードを生成してコピーまたは保存する
	  @return true:成功  false:失敗
	  @param[in] param アクションパラメーター
	  @param[out] errMsg エラーメッセージ
	*/
	bool Perform(Parameter* param, String* errMsg) override
	{
		UNREFERENCED_PARAMETER(param);
		QRCodeGenerator generator;
		if (generator.Generate(mText, mErrorCorrectionLevel) == false) {
			if (errMsg) {
				*errMsg = "QRコードを生成できません。文字数が多すぎるか、入力文字列が不正です。";
			}
			return false;
		}

		if (mShouldSave == false) {
			if (generator.CopyToClipboard()) {
				return true;
			}
			if (errMsg) {
				*errMsg = "QRコードをクリップボードにコピーできませんでした。";
			}
			return false;
		}

		CString filter;
		ATL::CSimpleArray<GUID> imageFileTypes;
		if (FAILED(ATL::CImage::GetExporterFilterString(filter, imageFileTypes))) {
			filter = _T("BMP (*.bmp)|*.bmp|GIF (*.gif)|*.gif|JPEG (*.jpg)|*.jpg|PNG (*.png)|*.png|TIFF (*.tif)|*.tif||");
		}
		filter.TrimRight(_T('|'));
		int filterSeparators = 0;
		for (int i = 0; i < filter.GetLength(); ++i) {
			if (filter[i] == _T('|')) {
				++filterSeparators;
			}
		}
		int svgFilterIndex = (filterSeparators + 1) / 2 + 1;
		filter += _T("|SVG (*.svg)|*.svg||");

		SharedHwnd owner;
		CFileDialog dialog(FALSE, nullptr, nullptr,
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
			filter, CWnd::FromHandle(owner.GetHwnd()));
		if (dialog.DoModal() != IDOK) {
			return true;
		}

		CString path = dialog.GetPathName();
		CString extension = PathFindExtension(path);
		if (extension.IsEmpty()) {
			if (dialog.m_ofn.nFilterIndex == static_cast<DWORD>(svgFilterIndex)) {
				path += _T(".svg");
			}
			else {
				int tokenIndex = 0;
				int wantedToken = (static_cast<int>(dialog.m_ofn.nFilterIndex) - 1) * 2 + 1;
				int position = 0;
				CString token = filter.Tokenize(_T("|"), position);
				while (position >= 0 && tokenIndex < wantedToken) {
					token = filter.Tokenize(_T("|"), position);
					++tokenIndex;
				}
				token.Replace(_T("*"), _T(""));
				int separator = token.Find(_T(';'));
				if (separator >= 0) {
					token = token.Left(separator);
				}
				if (token.IsEmpty()) {
					path += _T(".png");
				}
				else {
					path += token;
				}
			}
		}
		extension = PathFindExtension(path);
		bool saved = extension.CompareNoCase(_T(".svg")) == 0 ?
			generator.SaveSvg(path) : generator.SaveImage(path);
		if (saved) {
			return true;
		}
		if (errMsg) {
			*errMsg = "QRコードをファイルに保存できませんでした。";
		}
		return false;
	}

private:
	CString mText;
	int mErrorCorrectionLevel;
	bool mShouldSave;
};

} // 無名名前空間

struct QRCodeCommand::PImpl
{
	CString mPrefix;
	CString mText;
	int mErrorCorrectionLevel{QRCodeGenerator::LEVEL_M};
  HICON mIcon{nullptr};
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(QRCodeCommand)

QRCodeCommand::QRCodeCommand(const CString& prefix, const CString& text, int errorCorrectionLevel) :
	in(std::make_unique<PImpl>())
{
  in->mPrefix = prefix;
	in->mText = text;
	in->mErrorCorrectionLevel = errorCorrectionLevel;
	mName = text;
	mDescription = TypeDisplayName();
	HINSTANCE h = GetModuleHandle(nullptr);
	in->mIcon = LoadIcon(h, MAKEINTRESOURCE(IDI_QRCODE));
}

QRCodeCommand::~QRCodeCommand()
{
  DestroyIcon(in->mIcon);
}

CString QRCodeCommand::GetName()
{
	return in->mPrefix + _T(" ") + in->mText;
}

CString QRCodeCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool QRCodeCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	auto modifiers = hotkeyAttr.GetModifiers();
	if (modifiers == 0) {
		*action = new QRCodeAction(in->mText, in->mErrorCorrectionLevel, false);
		return true;
	}
	if (modifiers == MOD_CONTROL) {
		*action = new QRCodeAction(in->mText, in->mErrorCorrectionLevel, true);
		return true;
	}
	return false;
}

HICON QRCodeCommand::GetIcon()
{
  return in->mIcon;
}

launcherapp::core::Command* QRCodeCommand::Clone()
{
	return new QRCodeCommand(in->mPrefix, in->mText, in->mErrorCorrectionLevel);
}

launcherapp::mainwindow::preview::PreviewWindow* QRCodeCommand::FetchPreviewWindow()
{
	auto previewWindow = QRCodePreviewWindow::Get();
	if (previewWindow->SetCode(in->mText, in->mErrorCorrectionLevel) == false) {
		return nullptr;
	}
	return previewWindow;
}

bool QRCodeCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}
	if (ifid == IFID_PREVIEWPROVIDER) {
		AddRef();
		*cmd = static_cast<launcherapp::core::PreviewProvider*>(this);
		return true;
	}
	if (ifid == IFID_QRCODECOMMAND) {
		AddRef();
		*cmd = static_cast<QRCodeCommand*>(this);
		return true;
	}
	return false;
}

CString QRCodeCommand::TypeDisplayName()
{
	return _T("QRコード生成");
}

} // qrcode名前空間
} // commands名前空間
} // launcherapp名前空間
