#include "pch.h"
#include "AppSettingQRCode.h"
#include "commands/qrcode/QRCodeGenerator.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace qrcode {

class AppSettingQRCodeDialog : public CDialog
{
public:
	/**
	  設定ページを表示する前に設定を読み込む
	  @param[in] settings 設定の読み書き先
	*/
	void Load(Settings* settings)
	{
		mSettings = settings;
		mIsEnable = settings->Get(_T("QRCode:IsEnable"), true) ? 1 : 0;
		mPrefix = settings->Get(_T("QRCode:Prefix"), _T("qrcode"));
		mErrorCorrectionLevel = settings->Get(_T("QRCode:ErrorCorrectionLevel"), QRCodeGenerator::LEVEL_M);
		if (mErrorCorrectionLevel < QRCodeGenerator::LEVEL_L ||
			QRCodeGenerator::LEVEL_H < mErrorCorrectionLevel) {
			mErrorCorrectionLevel = QRCodeGenerator::LEVEL_M;
		}
	}

	/**
	  ページが非アクティブになる前に入力値を検証する
	  @return true:切り替え可能  false:入力値が不正
	*/
	bool Validate()
	{
		if (UpdateData() == FALSE) {
			return false;
		}
		if (mErrorCorrectionLevel < QRCodeGenerator::LEVEL_L ||
			QRCodeGenerator::LEVEL_H < mErrorCorrectionLevel) {
			AfxMessageBox(_T("誤り訂正レベルを選択してください。"));
			return false;
		}
		return true;
	}

	/**
	  入力値を設定へ保存する
	*/
	void Save()
	{
		if (mSettings == nullptr) {
			return;
		}
		mSettings->Set(_T("QRCode:IsEnable"), mIsEnable != 0);
		mSettings->Set(_T("QRCode:Prefix"), mPrefix);
		mSettings->Set(_T("QRCode:ErrorCorrectionLevel"), mErrorCorrectionLevel);
		__super::OnOK();
	}

	/**
	  ダイアログの値と設定値を関連付ける
	  @param[in] pDX データ交換情報
	*/
	void DoDataExchange(CDataExchange* pDX) override
	{
		__super::DoDataExchange(pDX);
		DDX_Check(pDX, IDC_QRCODE_ENABLE, mIsEnable);
		DDX_Text(pDX, IDC_QRCODE_PREFIX, mPrefix);
		DDX_CBIndex(pDX, IDC_QRCODE_ERROR_LEVEL, mErrorCorrectionLevel);
	}

	/**
	  コンボボックスを初期化する
	  @return TRUE:フォーカスを既定位置へ設定
	*/
	BOOL OnInitDialog() override
	{
		__super::OnInitDialog();
		auto level = static_cast<CComboBox*>(GetDlgItem(IDC_QRCODE_ERROR_LEVEL));
		level->AddString(_T("L"));
		level->AddString(_T("M"));
		level->AddString(_T("Q"));
		level->AddString(_T("H"));
		UpdateData(FALSE);
		UpdateStatus();
		return TRUE;
	}

	/**
	  有効状態に応じて設定項目を使用可能にする
	*/
	void UpdateStatus()
	{
		GetDlgItem(IDC_QRCODE_PREFIX)->EnableWindow(mIsEnable);
		GetDlgItem(IDC_QRCODE_ERROR_LEVEL)->EnableWindow(mIsEnable);
	}

	DECLARE_MESSAGE_MAP()
	/** 設定変更後にコントロールの有効状態を更新する */
	afx_msg void OnUpdateStatus()
	{
		UpdateData();
		UpdateStatus();
	}

private:
	Settings* mSettings{nullptr};
	int mIsEnable{1};
	CString mPrefix{_T("qrcode")};
	int mErrorCorrectionLevel{QRCodeGenerator::LEVEL_M};
};

BEGIN_MESSAGE_MAP(AppSettingQRCodeDialog, CDialog)
	ON_COMMAND(IDC_QRCODE_ENABLE, OnUpdateStatus)
END_MESSAGE_MAP()

struct AppSettingQRCode::PImpl
{
	AppSettingQRCodeDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingQRCode)

AppSettingQRCode::AppSettingQRCode() :
	AppSettingPageBase(_T("拡張機能"), _T("QRコード生成")),
	in(std::make_unique<PImpl>())
{
}

AppSettingQRCode::~AppSettingQRCode()
{
}

bool AppSettingQRCode::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_QRCODE, CWnd::FromHandle(parentWindow)) != FALSE;
}

HWND AppSettingQRCode::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

int AppSettingQRCode::GetOrder()
{
	return 470;
}

bool AppSettingQRCode::OnEnterSettings()
{
	in->mWindow.Load(static_cast<Settings*>(GetParam()));
	return true;
}

bool AppSettingQRCode::OnSetActive()
{
	in->mWindow.UpdateData(FALSE);
	in->mWindow.UpdateStatus();
	return true;
}

bool AppSettingQRCode::OnKillActive()
{
	return in->mWindow.Validate();
}

void AppSettingQRCode::OnOKCall()
{
	in->mWindow.Save();
}

bool AppSettingQRCode::GetHelpPageId(String& helpPageId)
{
	helpPageId = "QRCodeSetting";
	return true;
}

} // qrcode名前空間
} // commands名前空間
} // launcherapp名前空間
