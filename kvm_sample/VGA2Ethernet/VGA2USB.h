// VGA2USB.h : main header file for the VGA2USB application
//

#if !defined(AFX_VGA2USB_H__7AE1502B_B608_4250_9474_7DE3B5AF27B0__INCLUDED_)
#define AFX_VGA2USB_H__7AE1502B_B608_4250_9474_7DE3B5AF27B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


/////////////////////////////////////////////////////////////////////////////
// CVGA2USBApp:
// See VGA2USB.cpp for the implementation of this class
//

class CVGA2USBApp : public CWinApp
{
public:
	CVGA2USBApp();
	CString CVGA2USBApp::GetAppPath();
	void CVGA2USBApp::GetWindowConfig();

    BOOL m_bRecordStart;
    BOOL m_bRecordPause;
	CString strConfigFileName;
    BOOL m_bNetServiceStart;
	BOOL m_bNetDeviceConnected;
	
	int left;
	int top;
	int right;
	int bottom;
	int width;
	int height;
	
	BYTE ip0,ip1,ip2,ip3;
	int port;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVGA2USBApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL SaveAllModified(); // save before exit	
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CVGA2USBApp)
	afx_msg void OnAppAbout();
	afx_msg void OnAppSave();
	afx_msg void OnAppRecord();
	afx_msg void OnAppPause();
	afx_msg void OnFullScreen();
	afx_msg void OnConnectNetdevice();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VGA2USB_H__7AE1502B_B608_4250_9474_7DE3B5AF27B0__INCLUDED_)
