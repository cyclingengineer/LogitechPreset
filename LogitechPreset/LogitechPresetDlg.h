
// LogitechPresetDlg.h : header file
//

#include "xSkinButton.h"

#pragma once

class LogitechCamCtrl;

// CLogitechPresetDlg dialog
class CLogitechPresetDlg : public CDialog
{
// Construction
public:
	CLogitechPresetDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LOGITECHPRESET_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButtonRight();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButtonHome();
	afx_msg void OnBnClickedButtonLeft();
	afx_msg void OnBnClickedButtonUp();
	afx_msg void OnBnClickedButtonDown();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButtonNumber0();
	afx_msg void OnBnClickedButtonNumber1();
	afx_msg void OnBnClickedButtonNumber2();
	afx_msg void OnBnClickedButtonNumber3();
	afx_msg void OnBnClickedButtonNumber4();
	afx_msg void OnBnClickedButtonNumber5();
	afx_msg void OnBnClickedButtonNumber6();
	afx_msg void OnBnClickedButtonNumber7();
	afx_msg void OnBnClickedButtonNumber8();
public:
	LogitechCamCtrl *m_pLogitechCtrl;
	bool m_bInitSuccess;
	bool m_bSetPresetOn;
	afx_msg void OnBnClickedButtonPreset();
	afx_msg void OnBnClickedButtonReset();
	afx_msg void OnBnClickedGetValue();
};
