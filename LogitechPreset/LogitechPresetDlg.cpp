
// LogitechPresetDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LogitechPreset.h"
#include "LogitechPresetDlg.h"
#include "LogitechCamCtrl.h"
#include "colorBtn.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CLogitechPresetDlg dialog




CLogitechPresetDlg::CLogitechPresetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogitechPresetDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bInitSuccess = false;
	m_bSetPresetOn = false;
}

void CLogitechPresetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLogitechPresetDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_RIGHT, &CLogitechPresetDlg::OnBnClickedButtonRight)
	ON_BN_CLICKED(IDC_BUTTON_HOME, &CLogitechPresetDlg::OnBnClickedButtonHome)
	ON_BN_CLICKED(IDC_BUTTON_LEFT, &CLogitechPresetDlg::OnBnClickedButtonLeft)
	ON_BN_CLICKED(IDC_BUTTON_UP, &CLogitechPresetDlg::OnBnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, &CLogitechPresetDlg::OnBnClickedButtonDown)
	ON_BN_CLICKED(IDC_BUTTON_NUMBER_0, &CLogitechPresetDlg::OnBnClickedButtonNumber0)
	ON_BN_CLICKED(IDC_BUTTON_NUMBER_1, &CLogitechPresetDlg::OnBnClickedButtonNumber1)
	ON_BN_CLICKED(IDC_BUTTON_NUMBER_2, &CLogitechPresetDlg::OnBnClickedButtonNumber2)
	ON_BN_CLICKED(IDC_BUTTON_NUMBER_3, &CLogitechPresetDlg::OnBnClickedButtonNumber3)
	ON_BN_CLICKED(IDC_BUTTON_NUMBER_4, &CLogitechPresetDlg::OnBnClickedButtonNumber4)
	ON_BN_CLICKED(IDC_BUTTON_NUMBER_5, &CLogitechPresetDlg::OnBnClickedButtonNumber5)
	ON_BN_CLICKED(IDC_BUTTON_NUMBER_6, &CLogitechPresetDlg::OnBnClickedButtonNumber6)
	ON_BN_CLICKED(IDC_BUTTON_NUMBER_7, &CLogitechPresetDlg::OnBnClickedButtonNumber7)
	ON_BN_CLICKED(IDC_BUTTON_NUMBER_8, &CLogitechPresetDlg::OnBnClickedButtonNumber8)
	ON_BN_CLICKED(IDC_BUTTON_PRESET, &CLogitechPresetDlg::OnBnClickedButtonPreset)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CLogitechPresetDlg::OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_GET_VALUE, &CLogitechPresetDlg::OnBnClickedGetValue)
END_MESSAGE_MAP()


// CLogitechPresetDlg message handlers

BOOL CLogitechPresetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_pLogitechCtrl = new LogitechCamCtrl();
	if(m_pLogitechCtrl->open("QuickCam Orbit/Sphere AF"))
		m_bInitSuccess = true;
	else
		m_bInitSuccess = false;

	m_pLogitechCtrl->resetMechanicalPanTilt();

	ControlInfo info;

	if(m_bInitSuccess)
	{
		m_pLogitechCtrl->getControlInfo(LogitechCamCtrl::ControlInfoType::CIT_PAN, info);
		m_pLogitechCtrl->getControlInfo(LogitechCamCtrl::ControlInfoType::CIT_TILT, info);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLogitechPresetDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLogitechPresetDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLogitechPresetDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CLogitechPresetDlg::OnBnClickedButtonHome()
{
	// TODO: Add your control notification handler code here
	//m_pLogitechCtrl->resetMechanicalPanTilt();
	m_pLogitechCtrl->gotoHome();
}

void CLogitechPresetDlg::OnBnClickedButtonRight()
{
	// TODO: Add your control notification handler code here
	m_pLogitechCtrl->TurnRight();
}

void CLogitechPresetDlg::OnBnClickedButtonLeft()
{
	// TODO: Add your control notification handler code here
	m_pLogitechCtrl->TurnLeft();
}

void CLogitechPresetDlg::OnBnClickedButtonUp()
{
	// TODO: Add your control notification handler code here
	m_pLogitechCtrl->TurnUp();
}

void CLogitechPresetDlg::OnBnClickedButtonDown()
{
	// TODO: Add your control notification handler code here
	m_pLogitechCtrl->TurnDown();
}

void CLogitechPresetDlg::OnBnClickedButtonPreset()
{
	// TODO: Add your control notification handler code here
	m_bSetPresetOn = !m_bSetPresetOn;

	if(m_bSetPresetOn)
		SetDlgItemText(IDC_NOTIFY, "Preset on");
	else
		SetDlgItemText(IDC_NOTIFY, "Preset off");
}


void CLogitechPresetDlg::OnBnClickedButtonNumber0()
{
	// TODO: Add your control notification handler code here
	if(m_bSetPresetOn)
		m_pLogitechCtrl->setPresetPos(0);
	else
		m_pLogitechCtrl->gotoPresetPos(0);
	m_bSetPresetOn = false;
	SetDlgItemText(IDC_NOTIFY, "Preset off");
}

void CLogitechPresetDlg::OnBnClickedButtonNumber1()
{
	// TODO: Add your control notification handler code here
	if(m_bSetPresetOn)
		m_pLogitechCtrl->setPresetPos(1);
	else
		m_pLogitechCtrl->gotoPresetPos(1);
	m_bSetPresetOn = false;
	SetDlgItemText(IDC_NOTIFY, "Preset off");
}

void CLogitechPresetDlg::OnBnClickedButtonNumber2()
{
	// TODO: Add your control notification handler code here
	if(m_bSetPresetOn)
		m_pLogitechCtrl->setPresetPos(2);
	else
		m_pLogitechCtrl->gotoPresetPos(2);

	m_bSetPresetOn = false;
	SetDlgItemText(IDC_NOTIFY, "Preset off");
}

void CLogitechPresetDlg::OnBnClickedButtonNumber3()
{
	// TODO: Add your control notification handler code here
	if(m_bSetPresetOn)
		m_pLogitechCtrl->setPresetPos(3);
	else
		m_pLogitechCtrl->gotoPresetPos(3);

	m_bSetPresetOn = false;
	SetDlgItemText(IDC_NOTIFY, "Preset off");
}

void CLogitechPresetDlg::OnBnClickedButtonNumber4()
{
	// TODO: Add your control notification handler code here
	if(m_bSetPresetOn)
		m_pLogitechCtrl->setPresetPos(4);
	else
		m_pLogitechCtrl->gotoPresetPos(4);

	m_bSetPresetOn = false;
	SetDlgItemText(IDC_NOTIFY, "Preset off");
}

void CLogitechPresetDlg::OnBnClickedButtonNumber5()
{
	// TODO: Add your control notification handler code here
	if(m_bSetPresetOn)
		m_pLogitechCtrl->setPresetPos(5);
	else
		m_pLogitechCtrl->gotoPresetPos(5);
	
	m_bSetPresetOn = false;
	SetDlgItemText(IDC_NOTIFY, "Preset off");
}

void CLogitechPresetDlg::OnBnClickedButtonNumber6()
{
	// TODO: Add your control notification handler code here
	if(m_bSetPresetOn)
		m_pLogitechCtrl->setPresetPos(6);
	else
		m_pLogitechCtrl->gotoPresetPos(6);
	
	m_bSetPresetOn = false;
	SetDlgItemText(IDC_NOTIFY, "Preset off");
}

void CLogitechPresetDlg::OnBnClickedButtonNumber7()
{
	// TODO: Add your control notification handler code here
	if(m_bSetPresetOn)
		m_pLogitechCtrl->setPresetPos(7);
	else
		m_pLogitechCtrl->gotoPresetPos(7);

	m_bSetPresetOn = false;
	SetDlgItemText(IDC_NOTIFY, "Preset off");
}

void CLogitechPresetDlg::OnBnClickedButtonNumber8()
{
	// TODO: Add your control notification handler code here
	if(m_bSetPresetOn)
		m_pLogitechCtrl->setPresetPos(8);
	else
		m_pLogitechCtrl->gotoPresetPos(8);

	m_bSetPresetOn = false;
	SetDlgItemText(IDC_NOTIFY, "Preset off");
}


void CLogitechPresetDlg::OnBnClickedButtonReset()
{
	// TODO: Add your control notification handler code here
	if(m_pLogitechCtrl)
		m_pLogitechCtrl->resetMechanicalPanTilt();

	m_bSetPresetOn = false;
	SetDlgItemText(IDC_NOTIFY, "Preset off");
}

void CLogitechPresetDlg::OnBnClickedGetValue()
{
	// TODO: Add your control notification handler code here
	if(m_pLogitechCtrl)
		m_pLogitechCtrl->getValue();

}
