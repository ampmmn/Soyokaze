#include "pch.h"
#include "QRCodeCommandProvider.h"
#include "commands/qrcode/QRCodeCommand.h"
#include "commands/qrcode/QRCodeGenerator.h"
#include "commands/qrcode/QRCodePreviewWindow.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace qrcode {

struct QRCodeCommandProvider::PImpl : public AppPreferenceListenerIF
{
	/**
	  設定変更リスナーとして登録する
	*/
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this, _T("QRCodeCommandProvider"));
	}

	/**
	  設定変更リスナーを解除する
	*/
	~PImpl() override
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	/** 初回起動時のコールバック */
	void OnAppFirstBoot() override {}
	/** 通常起動時のコールバック */
	void OnAppNormalBoot() override {}
	/** 設定変更時に設定値を再読み込みする */
	void OnAppPreferenceUpdated() override
	{
		ReloadSettings();
	}
	/** アプリケーション終了時にプレビューウインドウを破棄する */
	void OnAppExit() override
	{
		QRCodePreviewWindow::Get()->Destroy();
	}

	/**
	  アプリケーション設定からQRコード生成の設定を読み込む
	*/
	void ReloadSettings()
	{
		auto settings = &AppPreference::Get()->GetSettings();
		mIsEnable = settings->Get(_T("QRCode:IsEnable"), true);
		mPrefix = settings->Get(_T("QRCode:Prefix"), _T("qrcode"));
		mErrorCorrectionLevel = settings->Get(_T("QRCode:ErrorCorrectionLevel"), QRCodeGenerator::LEVEL_M);
		if (mErrorCorrectionLevel < QRCodeGenerator::LEVEL_L ||
			QRCodeGenerator::LEVEL_H < mErrorCorrectionLevel) {
			mErrorCorrectionLevel = QRCodeGenerator::LEVEL_M;
		}
	}

	bool mIsEnable{true};
	CString mPrefix{_T("qrcode")};
	int mErrorCorrectionLevel{QRCodeGenerator::LEVEL_M};
};

REGISTER_COMMANDPROVIDER(QRCodeCommandProvider)

/**
  コマンドプロバイダーを初期化する
*/
QRCodeCommandProvider::QRCodeCommandProvider() : in(std::make_unique<PImpl>())
{
}

/**
  コマンドプロバイダーを破棄する
*/
QRCodeCommandProvider::~QRCodeCommandProvider()
{
}

/**
  プロバイダー名を取得する
  @return プロバイダー名
*/
CString QRCodeCommandProvider::GetName()
{
	return _T("QRCodeCommandProvider");
}

/**
  Adhocコマンド候補を作成する前に設定を読み込む
*/
void QRCodeCommandProvider::PrepareAdhocCommands()
{
	in->ReloadSettings();
}

/**
  入力キーワードと本文が一致するQRコード候補を追加する
  @param[in] pattern 入力パターン
  @param[out] commands 追加するコマンド候補一覧
*/
void QRCodeCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	if (in->mIsEnable == false || in->mPrefix.IsEmpty()) {
		return;
	}

	CString firstWord = pattern->GetFirstWord();
	if (firstWord.CompareNoCase(in->mPrefix) != 0) {
		return;
	}

	CString input = pattern->GetWholeString();
	int separator = input.FindOneOf(_T(" \t\r\n"));
	if (separator < 0) {
		return;
	}
	CString text = input.Mid(separator + 1);
	text.TrimLeft();
	if (text.IsEmpty()) {
		return;
	}

	commands.Add(CommandQueryItem(Pattern::FrontMatch,
		new QRCodeCommand(in->mPrefix, text, in->mErrorCorrectionLevel)));
}

/**
  提供するコマンド種別を列挙する
  @return 列挙したコマンド種別数
  @param[out] displayNames コマンド種別名一覧
*/
uint32_t QRCodeCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(QRCodeCommand::TypeDisplayName());
	return 1;
}

} // qrcode名前空間
} // commands名前空間
} // launcherapp名前空間
